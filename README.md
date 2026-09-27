# ECE 309 — Project 2: The Conversation Loop

This project implements the conversation memory and streaming components of a minimal LLM harness in C++.

The project uses the provided `Harness`, `ModelClient`, `ScriptedModelClient`, and `ReplayModelClient` implementations. The main components implemented for Project 2 are the `Conversation` growable array and the `SentinelScanner`.

## Features

- Stores System, User, and Assistant messages
- Custom dynamically allocated growable array without `std::vector`
- Rule of Five support for safe copying and moving
- Bounds-checked conversation access
- Amortized O(1) message insertion using 2x capacity growth
- Streaming detection of `<|end_conversation|>`
- Detects sentinels split across arbitrary stream chunks
- Bounded sentinel pending buffer
- Turn-limit and EOF handling through the provided harness
- Scripted model responses
- Transcript replay support
- Assert-based test suite

## Project Structure

- `include/core/` — Message, Conversation, and SentinelScanner headers
- `src/` — implementations and provided harness/model source files
- `tests/p2/` — Project 2 test suite
- `scripts/` — example model scripts
- `docs/` — Project 2 design log

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

This builds:

- `./build/miniharness` — interactive CLI
- `./build/test_p2` — Project 2 test suite

## Run

Run the harness using the provided greeting script:

```bash
./build/miniharness --script scripts/greeting.script
```

To save the conversation transcript:

```bash
./build/miniharness --script scripts/greeting.script --save transcript.txt
```

Press Ctrl-D to end the conversation through EOF.

A maximum number of turns can also be specified:

```bash
./build/miniharness --script scripts/greeting.script --max-turns 2
```

## Testing

Build the project and run:

```bash
./build/test_p2
```

The test suite checks Conversation behavior, deep-copy and move semantics, bounds checking, growable-array reallocation, sentinel detection across chunk boundaries, one-character streaming, large-stream behavior, harness turn limits, sentinel termination, and transcript replay.

The project can also be built and tested with AddressSanitizer enabled through the provided CMake configuration.