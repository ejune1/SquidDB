# SquidDB

A lightweight, persistent, append only, relational row store built from scratch in C++. This project utilizes a probabilistic **SkipList** as its primary indexing structure and storage engine, providing $O(\log n)$ average time complexity for data operations.

*Note: This is a personal project built for educational purposes*

## 🏗️ Architecture & Query Pipeline

SquidDB processes queries through a traditional relational database pipeline:

```mermaid
graph LR
    A[Client Connection] --> B[Network Head]
    B --> C[Lexer & Parser]
    C --> D[Query Planner]
    D --> E[Execution Engine]
    E --> F[Table Representation]
    F --> G[SkipList]
```

1. **Network Head:** Handles incoming client TCP connections.
2. **Lexer & Parser:** Tokenizes and parses a subset of SQL into an Abstract Syntax Tree (AST).
3. **Query Planner:** Evaluates the AST to determine the optimal retrieval strategy (e.g., Index Point Lookup vs. Sequential Scan).
4. **Execution Engine:** Executes the physical plan.
5. **Table Representation:** Contains table, persistence,and index operations externally visible.
6. **SkipList:** Data structure representing indexes (both primary and secondary).

## 🚀 Current Project Status

- [x] **Core Indexing:** SkipList insertion, removal, lookups, widths, iterators (in progress).
- [X] **Storage Layer:** Table and row data, log data, schema, recovery and startup (in progress).
- [ ] **Network Layer:** Multi-threaded TCP connection handling using basic sockets (planned).
- [ ] **SQL Frontend:** Lexer and parser for a subset of SQL queries (planned).
- [ ] **Execution Engine:** TBD (planned).

## Recently Completed

- Unit test infrastructure
- SkipList structure and basic operations (single threaded)
- Pointer widths (for rank and cardinality estimation)
- Switch from make to cmake

## Immediate Tasks (in no particular order)

- Schema create/read
- Basic Table class structure
- Row writes to disk
- Log writes to disk (WAL)
- Error recovery (and startup for now)
- File rollover (reorg)
- Remove inline deletes
- Epoch for safe node deletion in batches
- Purge for cache pressure
- Opaque keys (value and pointer for large keys)
- Support secondary keys via opaque keys
- CAS (compare and swap) for node add remove (lockless algorithms)
- MVCC (versioning for information (ACID) information requires transaction ID)
- Custom spin lock in node (if needed)
- Arena allocation for nodes / large keys (placement new?)
- Ref count info for multiple indexes (share infos)
- Serialize SkipList structure to disk for faster restarts

## 🛠️ Getting Started

### Prerequisites

* GCC (C++ 17+)
* CMake 3.12+
* Catch2 (included)

## Building and Running

This project uses the CMake build system. All compilation steps should be performed out-of-source within a `build` directory.

### 1. Project Configuration

Before compiling, generate the build files. Run this once from the project root:

```bash
cmake -B build -S .
```

### 2. Build Instructions

#### Build Everything (App + Tests)
To build both the production application and the test suite:
```bash
cmake --build build
```

#### Build Production Only
To compile only the production application executable:
```bash
cmake --build build --target squid
```

#### Build Test Suite Only
To compile only the tests:
```bash
cmake --build build --target test_runner
```

### 3. Running the Project

#### Run the Production Binary
```bash
./build/squid
```

#### Run the Test Suite (Verbose Mode)
Run the test binary directly to see full Catch2 output, including passing assertions:
```bash
./build/test_runner -s
```
