# EDF Root Cause Analysis

## Problem

EDF (Earliest Deadline First) was failing for:
- `4 800 200 100 60 5 0 edf` (burnout 800ms, 5 compiles, no cooldown)
- `4 700 200 100 60 2 0 edf` (burnout 700ms)
- Any EDF run with `dongle_cooldown=60` and burnout 600–900ms

## Root Cause

**Starvation on equal deadlines.** When multiple coders had the same deadline (e.g. all starting at `last_compile_start = start_time`), the priority heap broke ties using insertion order. Lower `coder_id`s tended to win repeatedly, so higher `coder_id`s were starved and missed their burnout deadlines.

## Fix

Implemented the EDF tie-breaker required by the evaluation sheet: **on equal deadlines, prefer higher `coder_id`**.

In `dongle.c`, the priority for EDF is now:

```c
priority = deadline * (num_coders + 1) + (num_coders + 1 - coder_id);
```

- Earlier deadline → lower `priority` → higher effective priority.
- Same deadline: higher `coder_id` → lower second term → higher effective priority.

This rotates access so no coder is consistently starved when deadlines are equal.

## Cooldown Effect

With `dongle_cooldown > 0`, each release adds delay before the next use. For 4 coders (compile 200, debug 100, refactor 60), the effective cycle is ~520ms plus cooldown. With cooldown 60ms, burnout should be **> ~900ms** to avoid flakiness; **1500ms** is recommended for stable tests.

## Boundary Summary (4 coders, 200 compile, 100 debug, 60 refactor)

| Cooldown | Min burnout (approx) | Recommended for tests |
|----------|----------------------|------------------------|
| 0        | ~520ms               | 700+ (700–800 may be flaky) |
| 60       | ~600ms               | 1000+ (1500 recommended)    |

On slower systems or under Valgrind, increase burnout further.

## Diagnostic Script

Run `coders/edf_diag.sh` to sweep burnout values and compare EDF vs FIFO with cooldown 60.
