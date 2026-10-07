*This project has been created as part of the 42 curriculum by khabouou.*

# Codexion

## Description

Codexion is a multithreaded simulation inspired by the classic **Dining
Philosophers** problem, re-themed around a team of coders sharing a limited
pool of USB "dongles" needed to compile quantum code.

Each coder sits in a circular co-working hub and cycles endlessly through
three states: **compiling**, **debugging**, and **refactoring**. Compiling
requires holding **two** dongles at once — the one on the coder's left and
the one on the coder's right — which are shared with its two neighbors.
Dongles also have a **cooldown** period after being released, during which
they cannot be taken again.

The goal of the project is to correctly simulate this system using POSIX
threads and mutexes, while guaranteeing:

- No data races on shared state.
- No deadlock, ever, regardless of timing.
- No duplicate acquisition of a dongle.
- Accurate, serialized logging of every state change.
- Precise burnout detection, reported within 10 ms of the real deadline.
- A choice of scheduling policy — **FIFO** or **EDF** (Earliest Deadline
  First) — deciding who gets priority when multiple coders compete for the
  same dongle, with **EDF guaranteeing no starvation** under feasible
  parameters.

If any coder fails to start a new compile within `time_to_burnout`
milliseconds of its last compile (or of the simulation's start), it "burns
out" and the whole simulation stops. The simulation also stops cleanly once
every coder has reached `number_of_compiles_required` successful compiles.

## Instructions

### Compilation

```bash
make
```

This builds the `codexion` binary using `-Wall -Wextra -Werror -pthread`.

Other Makefile targets:

```bash
make clean   # remove object files
make fclean  # remove object files and the binary
make re      # fclean + all
```

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are mandatory, non-negative integers (`dongle_cooldown` may be
`0`), except `scheduler`, which must be exactly `fifo` or `edf`. Invalid
arguments (negative numbers, non-integers, wrong argument count, or an
unknown scheduler) are rejected with an error message and the program exits
without starting a simulation.

**Example:**

```bash
./codexion 5 3000 300 100 100 3 500 edf
```

This runs 5 coders, each burning out after 3000 ms without compiling, with a
300 ms compile phase, 100 ms debug phase, 100 ms refactor phase, requiring 3
successful compiles per coder, a 500 ms dongle cooldown, and EDF scheduling.

### Output format

Every state change is logged as:

```
<timestamp_ms> <coder_id> <message>
```

where `<message>` is one of: `has taken a dongle`, `is compiling`, `is
debugging`, `is refactoring`, or `burned out`. Lines are never interleaved,
even under heavy concurrency.

## Blocking cases handled

**Deadlock prevention (Coffman's conditions).** The classic Dining
Philosophers deadlock — every coder picking up their left dongle and waiting
forever for their right one — is prevented by never allowing a coder to
hold one dongle while waiting for the other. Acquisition is strictly
**both-or-nothing**: a coder locks both dongles it needs together (always in
a fixed order — the lower-ID dongle first, then the higher-ID one — so that
circular wait, one of Coffman's four necessary conditions, can never occur),
checks that both are actually available, and only takes them as an atomic
pair. If either is not ready, it releases both locks and retries later
instead of holding one hostage.

**Starvation prevention.** A single shared priority queue (protected by one
mutex) tracks every coder currently waiting to compile. Before attempting
to physically take its dongles, a coder checks whether any other coder
*that it actually competes with* (i.e. shares one of its two dongles) has
better priority. Under `edf`, priority is the coder's absolute deadline
(`last_compile_start + time_to_burnout`), so the coder closest to burning
out is always served first among its rivals, guaranteeing liveness. Under
`fifo`, priority is a strictly increasing arrival counter. Exact ties
(common right at simulation start, before anyone has compiled) are broken
deterministically by coder ID, so ordering is always fully defined and
reproducible. A small, fixed startup offset for even-numbered coders breaks
perfect timing symmetry that would otherwise create a permanent
unbreakable "round-robin resonance" chain around the ring.

**Cooldown handling.** Each dongle stores the timestamp of its last
release. A dongle is only considered available once
`current_time() - last_released_time >= dongle_cooldown`, enforced every
time a coder checks whether it can take a dongle. Because the design uses
polling (lock, check, unlock, sleep a fraction of a millisecond, retry)
rather than condition-variable waiting, a coder waiting purely on cooldown
naturally re-checks on its own as time passes, with no explicit wake-up
signal required.

**Precise burnout detection.** A dedicated monitor thread continuously
checks each coder's `last_compile_start` against `time_to_burnout`, cycling
through all coders with a short sleep interval between checks, so that a
burnout is always detected and logged within 10 ms of the real deadline.
Coders that have already met `number_of_compiles_required` are excluded
from the check, so a coder that finished successfully is never falsely
reported as burned out later in the run.

**Log serialization.** All output goes through a single print function
guarded by one dedicated mutex, so concurrent `printf` calls from different
coder threads or the monitor thread can never interleave mid-line.

**Edge case: a single coder.** With `number_of_coders == 1`, a coder's
"left" and "right" dongle are the same physical object, so the normal
two-dongle acquisition logic (which locks two distinct mutexes) cannot be
used — it would lock the same mutex twice from the same thread. This case
is handled separately: the coder takes the single available dongle once
(since it can never obtain the second one it would need to compile) and
then waits, correctly burning out once its deadline passes, matching the
simulation's own resource constraints.

## Thread synchronization mechanisms

The implementation uses only `pthread_mutex_t` — no condition variables are
used for coordinating dongle acquisition. This was a deliberate design
choice after an earlier version, which combined a single shared
`pthread_cond_t` with multiple different mutexes across concurrently
waiting threads, violated POSIX's requirement that all threads waiting on a
given condition variable use the *same* associated mutex, causing
intermittent crashes. The current design instead uses short, bounded
polling: a waiting coder repeatedly locks, checks its condition, unlocks,
and sleeps briefly (`usleep(100)`) before retrying.

**Per-dongle mutex (`t_dongle.lock`).** Protects a single dongle's own
state (`is_available`, `last_released_time`). Two dongles are always locked
in a fixed, global order (lower ID first) whenever a coder needs both at
once, which is what makes the both-or-nothing acquisition deadlock-free:
no two coders can ever be holding one dongle each while waiting on the
other's lock, since every coder reaching for the same pair approaches them
in the same order.

**Shared queue mutex (`sim->queue_lock`).** Protects the single global
priority queue of coders currently waiting to compile. Used only for brief
bookkeeping (pushing/removing a coder, scanning for rivals), never held
while sleeping.

**Arrival counter mutex (`sim->arrival_lock`).** Protects the monotonically
increasing counter used to assign FIFO arrival order, so two coders
requesting at the same instant can never receive the same or
out-of-order arrival values.

**Shared simulation-state mutex (`sim->state_lock`).** Protects every field
read or written by more than one thread: `simulation_finished`, and each
coder's `last_compile_start` and `compile_count`. The monitor thread reads
these to detect burnout and completion; coder threads write them after
every successful compile. All reads and writes go through small helper
functions (`sim_is_finished`, `sim_set_finished`, and locked reads inside
`burnout_check`/`all_compile_done`) so that no raw, unprotected access to
these fields exists anywhere in the codebase — this was verified with
ThreadSanitizer (`-fsanitize=thread`) across many stress-test runs with
varying coder counts, cooldowns, and both schedulers, with zero data races
reported.

**Print mutex (`sim->p_lock`).** Guards every `printf` call so that no two
threads can ever interleave partial lines of output.

**How thread-safe communication between coders and the monitor is
achieved.** The monitor thread never directly signals coder threads, and
coder threads never directly signal the monitor. Instead, both communicate
exclusively through mutex-protected shared state: a coder thread updates
`last_compile_start`/`compile_count` under `state_lock` after each
successful compile; the monitor thread independently polls that same
state, also under `state_lock`, on a short interval. This avoids any
need for condition variables between the two roles and keeps the
monitor's view of the simulation always consistent with what coders
have actually reported, never a stale or torn value.

## Resources

- *The Dining Philosophers Problem* — E. W. Dijkstra, original problem
  formulation.
- POSIX Threads Programming (LLNL Tutorial) —
  https://hpc-tutorials.llnl.gov/posix/
- `man pthread_mutex_init`, `man pthread_mutex_lock`,
  `man pthread_cond_wait`, `man gettimeofday` — POSIX manual pages used as
  the primary reference for every threading primitive in this project.
- *Operating System Concepts* (Silberschatz, Galvin, Gagne) — background on
  Coffman's deadlock conditions and classic starvation/fairness scheduling
  (FIFO vs. EDF).
- ThreadSanitizer documentation —
  https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual — used
  throughout development to detect and confirm the elimination of data
  races.

**AI usage.** An AI assistant was used throughout as a
debugging and diagnosing intermittent,
timing-dependent bugs (data races, an uninitialized-mutex hang, a stale
burnout check, a POSIX condition-variable/mutex-pairing violation that
caused crashes), explaining the trade-offs between different
synchronization strategies (shared-lock vs. per-resource locking,
condition-variable waiting vs. polling), and helping restructure functions
to respect the project's 25-line norm. All resulting code was reviewed,
tested, and iterated on manually (including with ThreadSanitizer, Valgrind,
and AddressSanitizer).
