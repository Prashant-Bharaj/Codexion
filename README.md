*This project has been created as part of the 42 curriculum by prasingh.*

# Codexion

## Description

Codexion is a concurrency simulation where multiple coders (threads) compete for limited USB dongles in a circular co-working hub. Each coder needs two dongles simultaneously to compile quantum code, then cycles through debug and refactor phases. The goal is to coordinate access using POSIX threads, mutexes, and condition variables—preventing deadlock and burnout while ensuring fair scheduling (FIFO or EDF).

## Instructions

### Compilation

```bash
cd coders/
make
```

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
  time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

**Arguments:**

- `number_of_coders`: Number of coders and dongles
- `time_to_burnout` (ms): Max time since last compile start before burnout
- `time_to_compile` (ms): Compile duration (holding 2 dongles)
- `time_to_debug` (ms): Debug duration
- `time_to_refactor` (ms): Refactor duration
- `number_of_compiles_required`: Simulation stops when all coders reach this count
- `dongle_cooldown` (ms): Delay before a released dongle can be taken again
- `scheduler`: `fifo` or `edf`

**Constraints enforced by the program:**

- All times (`time_to_burnout`, `time_to_compile`, `time_to_debug`,
  `time_to_refactor`, `dongle_cooldown`) must be **≥ 60 ms**.
- `number_of_compiles_required` must be **≥ 1**.

**Basic example:**

```bash
./codexion 4 410 200 200 200 5 60 fifo
```

## Test scenarios

These are verified working test cases:

```bash
# 2 coders FIFO — baseline, alternating compiles
./codexion 2 2000 200 200 200 3 60 fifo

# 3 coders FIFO — sequential round-robin, one compile at a time
./codexion 3 1500 200 200 200 4 80 fifo

# 4 coders EDF — coders 1&3 and 2&4 compile in parallel pairs
./codexion 4 1200 200 200 200 4 80 edf

# Cooldown stress test — 500ms cooldown, verifies dongle unavailability
./codexion 2 2000 200 200 200 3 500 fifo

# Burnout trigger — cycle time exceeds burnout, should fire correctly
./codexion 3 400 200 200 200 5 80 fifo
```

## Feasibility analysis

Not all parameter combinations are valid. The simulation will always burn out
if `time_to_burnout` is shorter than the minimum time a coder must wait before
its next compile.

### Formula

The minimum cycle time for one coder depends on the topology:

```
min_cycle = time_to_compile + time_to_debug + time_to_refactor
          + (max_coders_waiting × (time_to_compile + dongle_cooldown))
```

`time_to_burnout` must be **strictly greater** than `min_cycle`.

### Topology constraints

Coders sit in a circle and each needs their two neighbouring dongles.
The maximum number of coders that can compile simultaneously is `floor(N/2)`,
so a coder may wait for up to `ceil(N/2) - 1` others before it gets a turn.

| N (coders) | Max simultaneous compilers | Max others to wait for |
|:---:|:---:|:---:|
| 1 | 0 — **always burns out** (only 1 dongle exists, need 2) | — |
| 2 | 1 | 1 |
| 3 | 1 | 2 |
| 4 | 2 | 1 |
| 5 | 2 | 2 |
| N | floor(N/2) | ceil(N/2) - 1 |

### Worked examples

**1 coder:** Hard-coded infeasible. The program requires 2 dongles to compile
but only 1 exists in a circle of 1. `acquire_two_dongles` returns early and
the coder always burns out. This is a degenerate edge case acknowledged by the
subject.

**4 coders, burnout=410ms, compile/debug/refactor=200ms, cooldown=60ms:**
```
min_cycle = 200 + 200 + 200 + 1×(200 + 60) = 860ms  >  410ms  → always burns out
```

**5 coders, burnout=850ms, compile=200ms, debug=200ms, refactor=100ms, cooldown=80ms:**
```
max_others = ceil(5/2) - 1 = 2
min_cycle  = 200 + 200 + 100 + 2×(200 + 80) = 1060ms  >  850ms  → always burns out
```

**3 coders, burnout=1500ms, compile=200ms, debug=200ms, refactor=200ms, cooldown=80ms:**
```
max_others = ceil(3/2) - 1 = 1
min_cycle  = 200 + 200 + 200 + 1×(200 + 80) = 880ms  <  1500ms  → feasible ✓
```

**Safe minimum `time_to_burnout` for common setups (compile=debug=refactor=200ms, cooldown=80ms):**

| N | Min safe burnout |
|:---:|:---:|
| 2 | > 680ms |
| 3 | > 880ms |
| 4 | > 680ms |
| 5 | > 1060ms |
| 10 | > 1260ms |

## Blocking cases handled

- **Deadlock prevention (Coffman's conditions):**
  - Each coder always requests its two neighbouring dongles as an **atomic pair**:
    either it acquires both, or neither.
  - For each pair, dongle mutexes are taken in a fixed global order
    (`min(left_idx, right_idx)` then `max(left_idx, right_idx)`), which breaks
    circular wait.

- **Starvation prevention (EDF + tie‑breaker):**
  - Under EDF, each request is prioritized by
    `deadline = last_compile_start + time_to_burnout`.
  - A per‑dongle min‑heap priority queue orders requests by
    `priority = deadline * (num_coders + 1) + (num_coders - coder_id)`,
    so the coder closest to burnout (and, on ties, lowest ID) gets served first
    at every dongle.

- **Cooldown handling and wake‑up logic:**
  - When a dongle is released, `cooldown_until` is set to
    `now + dongle_cooldown`.
  - Releasing a dongle calls `wake_all_dongles()`, which briefly locks each
    dongle mutex and broadcasts on its condition variable. Waiting coders wake
    up and re‑check their pair; they only proceed when both dongles are free
    and `now >= cooldown_until` for each.

- **Precise burnout detection:** A dedicated monitor thread checks approximately
  every 1 ms whether any coder has exceeded its burnout deadline. Burnout is
  detected well within the required 10 ms tolerance.

- **Log serialization:** All log output goes through `safe_log()`, which locks
  a `log_mutex` before `printf`. This prevents two messages from interleaving on
  a single line.

## Thread synchronization mechanisms

- **`pthread_mutex_t` (per dongle):** Each dongle has a mutex protecting its state (`cooldown_until`, `holder`) and its request queue. Coders lock the dongle mutex when acquiring or releasing.

- **`pthread_cond_t` (per dongle):**
  - Each dongle has a condition variable. Coders waiting for a pair call
    `pthread_cond_timedwait` on the lower‑indexed dongle’s condvar.
  - `dongle_release()` and `signal_stop()` call `wake_all_dongles()`, which,
    while holding each dongle’s mutex, broadcasts on every condvar so all
    waiters can re‑evaluate whether their pair is now available or the
    simulation should stop.

- **`pthread_mutex_t` (log_mutex):** Serializes all `printf` calls so log lines are atomic.

- **`pthread_mutex_t` (stop_mutex):** Protects `sim->stop` and `sim->num_coders_finished`. When burnout or success is detected, `signal_stop()` sets `stop = 1` and broadcasts on all dongle condvars so blocked coders wake and exit.

- **`pthread_mutex_t` (per coder_data):** Protects `last_compile_start` and `compile_count` so the monitor can safely read them while coders update them.

- **Race condition prevention:** The monitor never holds `stop_mutex` while
  locking `coder_data[i].mutex`, avoiding deadlock. Coders acquire dongles in
  a fixed order and only ever take a pair atomically, preventing circular wait
  and partial‑allocation races.

## Resources

- AI was used to clarify the subject, and discuss deadlock prevention strategies.

- [POSIX Threads (pthreads) - IEEE Std 1003.1](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html)
- [Mutex and condition variable usage](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html)
- [Earliest Deadline First scheduling](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [Coffman conditions (deadlock)](https://en.wikipedia.org/wiki/Deadlock#Necessary_conditions)
