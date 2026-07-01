# ==============================================================================
# Compiler Options
# ==============================================================================
CXX      = g++
CXXFLAGS = -O0 -g -std=c++17 -Wall -Wextra -Iinclude
TEST_FLAGS = -fsanitize=address -fno-omit-frame-pointer

# This empty variable will hold the sanitize flags ONLY during a test run
SAN_FLAGS =

# ==============================================================================
# Source and Object Files
# ==============================================================================
CORE_SRCS = src/utils/Configuration.cxx src/utils/Logger.cxx src/utils/StringUtils.cxx src/core/SkipListNode.cxx src/core/SkipList.cxx src/core/TraverseContext.cxx src/core/SkipListIterator.cxx src/storage/Column.cxx src/storage/Schema.cxx src/engine/Table.cxx
MAIN_SRC  = src/main.cxx
TEST_SRCS = tests/test_main.cxx tests/SkipListTest.cxx

CORE_OBJS = $(CORE_SRCS:src/%.cxx=obj/%.o)
MAIN_OBJ  = obj/main.o
TEST_OBJS = $(TEST_SRCS:tests/%.cxx=obj/tests/%.o)

TARGET      = bin/squid
TEST_TARGET = bin/test_runner

# ==============================================================================
# Build Rules
# ==============================================================================

# 1. Default rule: Builds only the pure production binary (No sanitizers)
$(TARGET): $(CORE_OBJS) $(MAIN_OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) $(CORE_OBJS) $(MAIN_OBJ) -o $(TARGET)

# 2. 'make all' rule: Explicitly builds both the binary and the test runner
all: $(TARGET) $(TEST_TARGET)

# 3. Rule to link test executable
$(TEST_TARGET): $(CORE_OBJS) $(TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) $(CORE_OBJS) $(TEST_OBJS) -o $(TEST_TARGET)

# 4. Compile production source files into obj/
obj/%.o: src/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) -c $< -o $@

# 5. Compile main production file separately
$(MAIN_OBJ): $(MAIN_SRC)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 6. Compile test files into obj/tests/
obj/tests/%.o: tests/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) -c $< -o $@

# 7. Shortcut to build and immediately run the test suite
# TARGET-SPECIFIC VARIABLE: If 'make test' is invoked, activate the sanitizer flags!
test: SAN_FLAGS = $(TEST_FLAGS)
test: $(TEST_TARGET)
	./$(TEST_TARGET) -s

# 8. Clean up all build artifacts
clean:
	rm -rf obj bin

.PHONY: all test clean
