# Why 240ms Slack Can Cause Burnout (Root Cause Analysis)

## The Slack Calculation

With **4 coders, 600ms burnout, 360ms cycle** (200 compile + 100 debug + 60 refactor):
- **Slack** = 600 - 360 = **240ms** (time from "ready to acquire" to deadline)

That seems plenty. But the slack is consumed by **waiting for dongles**, not just "idle" time.

---

## Root Cause 1: Two-Dongle Acquisition = Two Waits

A coder must acquire **two** dongles. They can block twice:

1. **First dongle**: might be free or held — wait up to 200ms (one compile)
2. **Second dongle**: after getting the first, might have to wait for the holder to finish — **another 200ms**

Worst case: **400ms** total wait for both dongles. So we need:
- `slack > 200ms` (for second dongle wait)
- 240ms leaves only **40ms** margin

Any scheduling jitter (usleep overshoot, context switches, VM/WSL) can push you over.

---

## Root Cause 2: Per-Dongle Queues, Not Global EDF

Each dongle has its **own** priority queue. Example:

- Coder 1 needs dongles 0 and 3
- Coder 1 gets dongle 0 (earliest deadline for dongle 0)
- For dongle 3, Coder 4 might have an earlier deadline
- Coder 4 gets dongle 3
- Coder 1 is **stuck** holding 0 and waiting for 3 for up to 200ms

So one coder can block another even when the blocked coder has an earlier deadline for another resource. The effective wait can be up to one full compile (200ms) for the second dongle.

---

## Root Cause 3: Lock Ordering + Contention

Coders always acquire in index order (0 before 3). Coder 1 holds 0, waits for 3. Coder 4 holds 2, waits for 3. They contend for dongle 3. The one with earlier deadline gets it. The other waits until the holder finishes — ~200ms.

---

## Root Cause 4: `usleep` and Scheduling Jitter

`usleep(200000)` can overshoot, especially under load or in VMs. A 200ms sleep might take 205ms. Over several cycles, that eats into the slack.

---

## Summary

| Factor | Effect |
|--------|--------|
| Second-dongle wait | Up to 200ms |
| 240ms slack | Only 40ms margin after worst-case wait |
| usleep / scheduling jitter | Can exceed 40ms |
| WSL/VM | Adds more timing variance |

**Conclusion:** 240ms slack is not enough once you account for worst-case wait for the second dongle (~200ms) plus jitter. Slack should be comfortably above 200ms, e.g. 300–400ms or more.

---

## Root Cause 5: Start Time Set Too Early (FIXED)

**Issue:** `start_time` was set at the beginning of `init_simulation`, before allocation and thread creation. Init (malloc, dongle setup, etc.) can take 5–50ms. The burnout clock was already ticking during that time, so coders effectively lost that time.

**Fix:** Set `start_time` and `last_compile_start` for all coders **immediately before** `pthread_create`, in `run_simulation`. The clock now starts when threads actually begin, not during init.

---

## Clock Considerations

- **gettimeofday()**: Wall clock; can be affected by NTP or manual time changes. Subject allows it.
- **CLOCK_MONOTONIC** (clock_gettime): Monotonic clock, not affected by system time adjustments. More robust, but not in the subject’s allowed function list.
