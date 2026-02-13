# Codexion Evaluation Checklist

Adapted from the philosopher evaluation sheet. Use this during peer evaluation.

---

## Mapping: Philosopher → Codexion

| Philosopher | Codexion |
|-------------|----------|
| Philosopher | Coder |
| Fork | Dongle |
| Eat (hold 2 forks) | Compile (hold 2 dongles) |
| Sleep | Debug + Refactor |
| time_to_die | time_to_burnout |
| time_to_eat | time_to_compile |
| time_to_sleep | time_to_debug + time_to_refactor |
| num_times_must_eat | number_of_compiles_required |

---

## Mandatory Part

### Error Handling

- [ ] Code complies with the Norm
- [ ] No crash, undefined behavior, memory leak, or norm error (0 if found)

### Global Variables

- [ ] No global variables used to manage shared resources (dongles, coders, etc.)
- [ ] If found, evaluation stops here

### Codexion Code

Ensure the code complies with the following and ask for explanations:

- [ ] **One thread per coder** — each coder is a separate pthread
- [ ] **One dongle per coder** — num_dongles == num_coders
- [ ] **Mutex per dongle** — each dongle has a mutex protecting holder, cooldown, request queue
- [ ] **Outputs never mixed up** — log serialization via log_mutex
- [ ] **Burnout detection** — dedicated monitor thread; mutex to prevent race between burnout detection and coder starting compile

### Codexion Testing

**Rules:**
- Do not test with more than 200 coders
- Do not test with time params set to values lower than 60 ms

**Tests (run `./tester.sh` or manually):**

| Test | Args | Expected |
|------|------|----------|
| 1 | `1 800 200 200 60 5 0 fifo` | Coder should not compile, should burn out |
| 2 | `5 3000 200 200 60 5 0 fifo` | No burnout |
| 3 | `5 3000 200 200 60 7 0 fifo` | No burnout; simulation stops when all compiled 7 times |
| 4 | `4 2000 200 200 60 5 0 fifo` | No burnout |
| 5 | `4 310 200 100 60 5 0 fifo` | One coder should burn out |
| 6 | `2 200 200 200 60 5 0 fifo` | Burnout message within 10 ms of actual burnout |

*All time params (burnout, compile, debug, refactor) are kept ≥ 60 ms per eval rules.*

- [ ] Run additional tests to verify: burnout at right time, no dongle stealing, log format correct
- [ ] **EDF verification:** Run with `edf` scheduler; coders closest to burnout get dongles first

---

## Quick Run

```bash
cd coders/
./tester.sh
```
