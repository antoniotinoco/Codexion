__This project has been created as part of the 42 curriculum by atinoco-.__

# Codexion

## Description

Codexion is a concurrency project based on the Dining Philosophers problem, reworked around a group of coders competing for shared USB dongles while racing against a burnout deadline.

Each coder is represented by a POSIX thread. To compile, a coder must acquire two shared dongles, use them for the configured compilation time, then release them. Between compilations, coders debug and refactor. A monitor thread supervises the simulation and stops it when either a coder burns out or every coder has completed the required number of compilations.

The project adds a scheduling component to the classic problem. Waiting coders can be selected using either:

- **FIFO (First In, First Out):** the coder that has been waiting longest is selected first.
- **EDF (Earliest Deadline First):** the coder with the closest burnout deadline is selected first.

The implementation also focuses on concurrency correctness: mutual exclusion, condition variables, deadlock prevention, fairness, cooldown periods, precise timing, serialized logging, and safe resource cleanup.

## Instructions

### Requirements

The project is written in C and uses POSIX threads. A POSIX-compatible Unix-like environment is therefore required.

### Compilation

The project is compiled with the following flags:

```make
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
```

- `-Wall` enables common compiler warnings.
- `-Wextra` enables additional warnings.
- `-Werror` treats warnings as compilation errors.
- `-pthread` enables POSIX thread support.



The project is intended to be built with the provided `Makefile`:

```bash
make
```

The resulting executable is:

```text
./codexion
```

To clean object files:

```bash
make clean
```

To remove generated objects and the executable:

```bash
make fclean
```

To rebuild from scratch:

```bash
make re
```

### Execution

The program expects eight user-provided arguments:

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

| Parameter | Description |
|---|---|
| `number_of_coders` | Number of coders (1 to N) |
| `time_to_burnout` | Maximum time in milliseconds since compile start before burnout |
| `time_to_compile` | Duration of the compile phase in milliseconds |
| `time_to_debug` | Duration of the debug phase in milliseconds |
| `time_to_refactor` | Duration of the refactor phase in milliseconds |
| `number_of_compiles_required` | Number of compiles required per coder for success |
| `dongle_cooldown` | Cooldown period after a dongle is released, in milliseconds (0 allowed) |
| `scheduler` | Scheduling policy: `fifo` or `edf` |

Example:

```bash
./codexion 5 1500 200 200 100 5 50 fifo
```

The parser validates the number of arguments, rejects invalid numeric values, checks for integer overflow, and validates the scheduler name.

Quick log check: 
```bash
./codexion 5 1500 200 200 100 5 50 fifo | grep -c "is compiling"
```

You can use this to quickly check if the total number of compilations has been achieved by all coders.

### Expected log format

```text
0 2 has taken a dongle
0 2 has taken a dongle
0 2 is compiling
200 2 is debugging
210 1 has taken a dongle
210 1 has taken a dongle
210 1 is compiling
300 2 is refactoring
410 1 is debugging
420 2 has taken a dongle
420 2 has taken a dongle
420 2 is compiling
510 1 is refactoring
630 1 has taken a dongle
630 1 has taken a dongle
630 1 is compiling
```

Each line follows:

```text
timestamp_in_ms coder_id message
```

The timestamp is the elapsed time in milliseconds since the simulation started.

### Test file

The project includes a `test.py` script with a collection of automated tests covering different scenarios, including:

- Invalid arguments
- Single, two, three, and twenty coders
- Impossible configurations
- Dongle cooldown scenarios
- Stress tests
- Large thread counts
- Thread creation failures
- Graceful error handling

The test suite is also useful if you want to experiment with different configurations or add your own test cases. To run it:

```bash
python3 test.py
```

Make sure the Codexion executable has been built before running the test suite.

## Simulation lifecycle

At startup, the program:

1. Allocates the main simulation structure.
2. Parses and validates the arguments.
3. Allocates coder and dongle arrays.
4. Initializes mutexes and condition variables.
5. Initializes every coder and dongle.
6. Creates the coder threads.
7. Creates the monitor thread.
8. Releases the start barrier and records a common start timestamp.
9. Runs the simulation until the goal is reached or a coder burns out.
10. Joins all created threads.
11. Destroys synchronization objects and frees allocated memory.

## Concurrency Model

The program has two types of worker threads:

- **Coder threads:** repeatedly acquire two dongles, compile, release the dongles, debug, and refactor.
- **Monitor thread:** periodically checks whether all coders completed the required number of compilations or whether a coder has exceeded the burnout deadline.

The main thread is responsible for setup, thread creation, joining, and cleanup.

The shared `t_sim` structure contains configuration, coder and dongle arrays, synchronization primitives, timing information, and simulation state. Passing a pointer to this structure avoids global shared variables.

## Blocking cases handled

### Deadlock prevention

The underlying Dining Philosophers problem has four Coffman conditions. Named after computer scientist Edward G. Coffman Jr., these are conditions that must all be present for a deadlock to occur:

1. Mutual exclusion — a resource can only be held by one thread at a time.
2. Hold and wait — a thread holds one resource while waiting for another.
3. No preemption — a resource cannot be forcibly taken away from a thread.
4. Circular wait — threads form a cycle where each is waiting for a resource held by another thread.

Mutual exclusion is necessary because a dongle cannot be shared by two coders at once. The implementation breaks the circular-wait pattern by making acquisition asymmetric:

- Even-numbered coders acquire **left then right**.
- Odd-numbered coders acquire **right then left**.

This prevents every coder from taking the same side first and forming the classic circular deadlock.

### Resource ownership

Each dongle has its own `pthread_mutex_t`. A coder must hold the dongle mutex while checking and changing its availability and waiting queue.

The acquisition procedure only allows a coder to take a dongle when:

- The dongle is available.
- The coder is first in the scheduler's waiting queue.
- The dongle cooldown has expired.
- The simulation is still running.

This prevents two coders from claiming the same dongle simultaneously.

### Starvation and fairness

Waiting coders are registered in a per-dongle queue. The selected coder is determined by the configured scheduling policy:

- FIFO prioritizes arrival time.
- EDF prioritizes the earliest burnout deadline.
- EDF ties are resolved using coder ID.

This gives waiting coders a deterministic arbitration rule when two coders are queued on the same dongle. If a dongle is already free and uncontested, the first coder to reach it takes it immediately without entering the scheduler queue.

### Cooldown handling

A released dongle records its release timestamp. A new owner cannot immediately reuse it while the configured `dongle_cooldown` interval is active.

While waiting, the coder releases the dongle mutex, sleeps briefly, and re-locks it on each iteration. Cooldown is checked inside the same loop as availability and queue position, so a dongle cannot be taken before its cooldown expires.

### Precise burnout detection

The monitor checks the simulation every 2 ms. It compares the current timestamp with each unfinished coder's `last_compile_start`.

A coder is considered burned out when:

```text
current_time - last_compile_start > time_to_burnout
```

The shared coder state is protected by `sim_lock` while it is inspected.

### Fast simulation shutdown

When the monitor detects burnout or completion, it sets the shared `running` state to false while holding `sim_lock`.

It then broadcasts on the per-dongle condition variables. Coders blocked in `wait_for_dongle()` also re-check `sim_is_running()` on every polling iteration, so they exit quickly after shutdown.

The same stop state is checked by the coder loop and by the interruptible sleep helper, preventing threads from continuing unnecessary work after termination.

### Race-condition prevention

Shared state such as:

- `running`
- `start_time_ms`
- `last_compile_start`
- `compiles_done`
- burnout information

is accessed under `sim_lock`.

For example, the monitor cannot safely set `running = 0` at the same time that a coder reads `running`; both operations are serialized through the simulation mutex.

### Log serialization

All state output is protected by `log_lock`. This prevents concurrent calls to `printf()` from interleaving their output.

The logging path also checks the simulation state before printing, so messages are not emitted after the simulation has already stopped.

### Cleanup and partial initialization

Initialization can fail at several points: allocation, mutex creation, condition-variable creation, or thread creation.

The implementation tracks which synchronization objects and resources have actually been initialized. Cleanup therefore destroys only valid objects and frees allocated arrays without attempting to destroy uninitialized synchronization primitives.

Threads are joined before their associated synchronization objects and memory are destroyed.

## Thread synchronization mechanisms

### `pthread_mutex_t`

The implementation uses multiple mutexes with separate responsibilities:

- **`sim_lock`** protects simulation-wide shared state such as `running`, compile counters, timestamps, and burnout information.
- **`log_lock`** serializes terminal output.
- **`start_lock`** protects the start barrier and registration state.
- **Per-dongle mutexes** protect dongle availability, cooldown timestamps, and waiting queues.

A typical race prevention pattern is:

```c
pthread_mutex_lock(&sim->sim_lock);
coder->last_compile_start = current_time_ms();
pthread_mutex_unlock(&sim->sim_lock);
```

The monitor uses the same mutex when reading the timestamp. This establishes a synchronized access to the shared value.

### `pthread_cond_t`

Condition variables allow threads to sleep until a shared condition changes instead of continuously polling.

During startup, coder and monitor threads wait on `start_cond` until the main thread releases the start barrier:

```c
while (!sim->start_released && !sim->start_aborted)
    pthread_cond_wait(&sim->start_cond, &sim->start_lock);
```

For dongle acquisition, waiting uses a mutex-protected polling loop. The coder re-checks availability, queue position, and cooldown on each iteration:

```c
while (sim_is_running(sim))
{
    if (dongle->available
        && dongle->queue[0].coder_id == coder->id
        && current_time_ms() - dongle->timestamp
            >= sim->config.dongle_cooldown)
        break ;
    pthread_mutex_unlock(&dongle->lock);
    usleep(100);
    pthread_mutex_lock(&dongle->lock);
}
```

Each dongle still owns a condition variable. It is broadcast when the dongle is released and when the simulation stops, but the current wait loop relies on brief sleeps between re-checks rather than `pthread_cond_wait()`.

### Start barrier and thread-safe communication

The start barrier uses `start_lock` together with `start_cond`.

Threads first wait until `start_released` becomes true. The releasing code records a common `start_time_ms`, initializes the coders' timing state, sets the release flag, and broadcasts to all waiting threads.

This guarantees that all threads become eligible to start from the same logical simulation timestamp, while still allowing the operating-system scheduler to decide the exact order in which they execute.

### Monitor-to-coder communication

The monitor communicates termination through shared state and condition-variable broadcasts:

1. The monitor detects burnout or completion.
2. It updates `running` while holding `sim_lock`.
3. It broadcasts on each dongle condition variable.
4. Coders in `wait_for_dongle()`, `sleep_with_stop()`, or the main coder loop re-check `sim_is_running()` or `coder_should_continue()`.
5. Their loops terminate without continuing the simulation.

This avoids leaving threads blocked indefinitely after the simulation has ended.

## Scheduling: FIFO vs. EDF

### FIFO

FIFO orders waiting coders according to `arrival_time`, meaning the coder that started waiting first gets priority.

With the documented queue size, the implementation compares the two waiting requests and swaps them when their arrival order is reversed.

### EDF

EDF calculates a coder's deadline as:

```text
deadline = last_compile_start + time_to_burnout
```

The waiting coder with the smallest deadline has priority.

When deadlines are equal, the smaller coder ID wins. This makes the scheduler deterministic.

### Per-dongle waiting queue

Each dongle keeps a small waiting queue with room for at most two coders. In a ring of N coders, a given dongle can be needed by at most its two neighbors, so a fixed two-slot queue is enough.

When a second coder joins the queue, the implementation reorders the two entries according to the active policy:

- **FIFO:** swap if the earlier arrival time is not already at the front.
- **EDF:** swap if the earlier deadline is not already at the front; equal deadlines break on smaller coder ID.

Only the coder at `queue[0]` may take the dongle once it is available and its cooldown has expired. The scheduling logic is implemented in `scheduler_queue.c`.

## Timing

The simulation uses `gettimeofday()` to obtain timestamps with millisecond precision.

Elapsed time is calculated from the simulation start time:

```text
elapsed = current_time_ms - start_time_ms
```
These timestamps are used for:
- measuring compile, debug, and refactor durations;
- tracking when a coder last started compiling;
- calculating EDF deadlines;
- tracking dongle cooldown periods;
- detecting burnout.

The monitor checks the state of the simulation every 2 milliseconds, allowing it to detect burnout and completion without continuously consuming CPU.

## Resources

### Concurrency and synchronization

- [POSIX Threads Programming — Blaise Barney, LLNL](https://hpc-tutorials.llnl.gov/posix/)
- [Operating Systems: Three Easy Pieces — Arpaci-Dusseau](https://pages.cs.wisc.edu/~remzi/OSTEP/) (chapters on concurrency)
- [Linux timeval type documentation](https://man7.org/linux/man-pages/man3/timeval.3type.html)
- [The Dining Philosophers in C: threads, race conditions and deadlocks](https://www.youtube.com/watch?v=zOpzGHwJ3MU)
- [Philosophers, 42 School Project. Dining Philosophers Project. C Implementation](https://www.youtube.com/watch?v=UGQsvVKwe90)
- [Earliest Deadline First — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)

## AI usage

AI was used as a development assistant throughout this project to discuss concepts and help diagnose bugs. It also helped organize my many notes, turning them into a concise README structure.
