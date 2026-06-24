# Compiler and compilation flags
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# List your source files here manually
SRCS = src/main.cxx src/utils/Configuration.cxx src/utils/Logger.cxx src/utils/StringUtils.cxx src/core/SkipListNode.cxx src/core/SkipList.cxx

# Converts "src/utils/logger.cpp" into "obj/utils/logger.o"
OBJS = $(SRCS:src/%.cxx=obj/%.o)

# The final path of your executable inside the bin folder
TARGET = bin/squid

# 1. Default rule
all: $(TARGET)

# 2. Rule to link object files into the final executable inside bin/
$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# 3. Rule to compile individual .cpp files into the obj/ folder
obj/%.o: src/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Rule to clean up all build artifacts
clean:
	rm -rf obj bin
