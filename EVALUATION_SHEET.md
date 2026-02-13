# Codexion Evaluation Sheet

## Basics

- [ ] This project must be coded in **C89**, compiling with `-Wall -Wextra -Werror -pthread` without any warnings.
- [ ] Any of the following means **0** to the project:
  - Compilation error
  - Segmentation fault
  - Data race
  - Deadlock
  - Interleaved log line
  - Memory leak
- [ ] The program must take exactly the following arguments, in order:
  - `number_of_coders`, `time_to_burnout`, `time_to_compile`, `time_to_debug`, `time_to_refactor`, `number_of_compiles_required`, `dongle_cooldown`, `scheduler`
- [ ] Each coder must be represented by a thread (using `pthread_create`).
- [ ] The scheduler must be either `"fifo"` or `"edf"`.
- [ ] On some slow hardware, precise timing may vary slightly; discuss borderline cases honestly before counting them as failures.
- [ ] Verify compilation: `make`
- [ ] The binary must be named `codexion`.

## Global

- [ ] Check if there is any **global mutable variable** used to manage:
  - Shared resources (dongles)
  - Scheduling policy
  - Logging
- [ ] If found, **evaluation stops here**.

## README.md File

Does the repository contain a `README.md` at its root with:

- [ ] First line italicized and formatted exactly as: *This project has been created as part of the 42 curriculum by \<login1\>[, \<login2\>, \<login3\>[...]]].*
- [ ] A **"Description"** section explaining the project's purpose and providing a brief overview.
- [ ] An **"Instructions"** section with relevant details about compilation, installation, and/or execution.
- [ ] A **"Resources"** section listing references (documentation, tutorials, etc.) and explaining how AI was used.
- [ ] A **"Blocking cases handled"** section describing all the concurrency issues addressed.
- [ ] A **"Thread synchronization mechanisms"** section explaining the threading primitives used.

## Codexion Testing — Easy

- [ ] Do not test with more than 200 coders.
- [ ] Do not test with timing values under 60 ms.
- [ ] Verify no coder burns out and all complete required compiles.

## Codexion Testing — Less Easy

- [ ] Validate burnout edge cases.
- [ ] Verify correct logging and timing tolerances.
- [ ] Check no dongle duplication, correct state transitions.

## Codexion Testing — Medium

- [ ] Verify cooldown behavior.
- [ ] Test scheduler differences (EDF vs FIFO).
- [ ] Validate refactoring timing and log serialization.

## Codexion Recode — EDF Tie-Breaker

- [ ] Modify EDF to prefer **higher coder_id** on equal deadlines.
- [ ] Validate via contention tests.
