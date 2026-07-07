cmake_minimum_required(VERSION 3.12)
project(SquidProject LANGUAGES CXX)

# 1. Global Compiler Options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apply common warning flags
add_compile_options(-Wall -Wextra -Wold-style-cast)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type" FORCE)
endif()

# 2. Source Files Definition
set(CORE_SRCS
    src/utils/Configuration.cxx
    src/utils/Logger.cxx
    src/utils/StringUtils.cxx
    src/core/RowInfo.cxx
    src/core/SkipListNode.cxx
    src/core/SkipList.cxx
    src/core/TraverseContext.cxx
    src/core/SkipListIterator.cxx
    src/storage/Column.cxx
    src/storage/Schema.cxx
    src/engine/Table.cxx
)

set(MAIN_SRC 
	src/main.cxx
)

set(TEST_SRCS
    tests/test_main.cxx
    tests/SkipListTest.cxx
    tests/SchemaTest.cxx
)

# 3. Production Binary (squid)
add_executable(squid ${MAIN_SRC} ${CORE_SRCS})
target_include_directories(squid PRIVATE include)

# 4. Test Binary (test_runner)
add_executable(test_runner ${TEST_SRCS} ${CORE_SRCS})
target_include_directories(test_runner PRIVATE include)

# Apply AddressSanitizer flags ONLY to the test runner
target_compile_options(test_runner PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(test_runner PRIVATE -fsanitize=address)

enable_testing()
add_test(NAME SquidTests COMMAND test_runner)
