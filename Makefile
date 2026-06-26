# ==============================================================================
# Compiler Options
# ==============================================================================
CXX      = g++
CXXFLAGS = -O0 -g -std=c++17 -Wall -Wextra -Iinclude

# ==============================================================================
# Source and Object Files
# ==============================================================================
# Production core logic (Everything EXCEPT your main.cxx)
CORE_SRCS = src/utils/Configuration.cxx src/utils/Logger.cxx src/utils/StringUtils.cxx src/core/SkipListNode.cxx src/core/SkipList.cxx
MAIN_SRC  = src/main.cxx

# Test runner and individual unit test files
TEST_SRCS = tests/test_main.cxx tests/SkipListTest.cxx

# Target object file mapping
CORE_OBJS = $(CORE_SRCS:src/%.cxx=obj/%.o)
	MAIN_OBJ  = obj/main.o
	TEST_OBJS = $(TEST_SRCS:tests/%.cxx=obj/tests/%.o)

# Executable output paths
TARGET      = bin/squid
TEST_TARGET = bin/test_runner

# ==============================================================================
# Build Rules
# ==============================================================================

# 1. Default rule (runs when you type just 'make'): Builds only the production binary
$(TARGET): $(CORE_OBJS) $(MAIN_OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(CORE_OBJS) $(MAIN_OBJ) -o $(TARGET)

# 2. 'make all' rule: Explicitly builds both the binary and the test runner
all: $(TARGET) $(TEST_TARGET)

# 3. Rule to link test executable (Core logic + test objects, omits main.o)
$(TEST_TARGET): $(CORE_OBJS) $(TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(CORE_OBJS) $(TEST_OBJS) -o $(TEST_TARGET)

# 4. Compile production source files into obj/
obj/%.o: src/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5. Compile test files into obj/tests/
obj/tests/%.o: tests/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 6. Shortcut to build and immediately run the test suite
test: $(TEST_TARGET)
	./$(TEST_TARGET) -s

# 7. Clean up all build artifacts
clean:
	rm -rf obj bin

# Declare non-file targets to prevent filesystem collisions
.PHONY: all test clean
