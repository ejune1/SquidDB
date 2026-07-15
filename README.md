# SquidDB

SquidDB is a lightweight database engine built in pure C++. It is persistent, using a combination of append only files: A WAL (write ahead log), value (row) files, and index (checkpoint) files. It is transactional, utilizing MVCC (multi version concurrency control) to enable standard isolation level views for transactions. SquidDB indexes are based on a SkipList, providing $O(\log n)$ average time complexity for data operations. SquidDB is a row store, not a key/value store, meaning the keys themselves are extracted from the rows (the values).  

*Note: This is a personal project built for educational purposes*

## 🏗️ Architecture

SquidDB processes requests through a Table abstraction of the underlying data structures. The Table owns properties such as the schema, file manager, and indexes. The indexes (both primary and secondary) are based on a SkipList and are fully managed by the Table object. SquidDB offers custom configuration and custom logging. A Squiddb head (SQL parser and execution engine) is planned based on completion of the engine.  

## 🚀 Current Project Status

- [x] **Core Indexing:** SkipList insertion, removal, lookups, widths, iterators (in progress).
- [X] **Storage Layer:** Table and row data, log data, schema, recovery and startup (in progress).
- [ ] **Transactions:** MVCC supporting the 4 basic isolations levels for transactions (TBD).
- [ ] **Multi-Threaded Support:** Fine grained locking supporting multiple clients (TBD).
- [ ] **Extensive Testing:** Unit, integration, and system level testing and examples utilizing Catch2 framework (TBD).

## Recently Completed

- Schema write/read
- Basic Table class structure
- Transaction layout
- FileManager implementation

## Immediate Tasks (in no particular order)

- Row writes to disk
- Log writes to disk (WAL)
- Error recovery (and startup for now)
- File rollover (reorg)
- Remove inline deletes
- Purge for cache pressure
- Transactions
- Cross table transactions
- Epoch for safe node deletion in batches
- Opaque keys (value and pointer for large keys)
- Support secondary keys via opaque keys
- CAS (compare and swap) for node add remove (lockless algorithms)
- MVCC (versioning for information (ACID) information requires transaction ID)
- Custom spin lock in node (if needed) for write-write and background threads
- Arena allocation for nodes / large keys (placement new?)
- Ref count info for multiple indexes (share infos)
- Serialize SkipList structure to disk for faster restarts (checkpointing)

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
