cmake_minimum_required(VERSION 3.12)
project(SquidDB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apply common warning flags across all targets
add_compile_options(-Wall -Wextra -Wold-style-cast)

# Default to Debug build if no type is explicitly defined
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type" FORCE)
endif()

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
	src/core/KeyRowInfo.cxx
	src/core/Transaction.cxx
	src/core/ThreadContext.cxx
	src/core/ThreadContextManager.cxx
	src/storage/StreamFile.cxx
	src/storage/FileManager.cxx
)

set(MAIN_SRC 
	src/main.cxx
)

set(UNIT_TEST_SRCS
	tests/unit/test_main.cxx
	tests/unit/SkipListTest.cxx
	tests/unit/SchemaTest.cxx
	tests/unit/TableTest.cxx
	tests/unit/FileManagerTest.cxx
	tests/unit/ThreadContextManagerTest.cxx
)

set(INTEGRATION_TEST_SRCS
	tests/integration/test_main.cxx
	tests/integration/InsertTest.cxx
)

set(PERF_TEST_SRCS
	tests/performance/test_main.cxx
)

add_library(squid_core STATIC ${CORE_SRCS})
target_include_directories(squid_core PUBLIC include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_library(squid_core_release STATIC ${CORE_SRCS})
	target_include_directories(squid_core_release PUBLIC include)
	target_compile_options(squid_core_release PRIVATE -O3 -DNDEBUG -fomit-frame-pointer)
endif()

add_executable(squid ${MAIN_SRC})
target_link_libraries(squid PRIVATE squid_core)

add_executable(unit_test_runner ${UNIT_TEST_SRCS})
target_link_libraries(unit_test_runner PRIVATE squid_core)

target_compile_options(unit_test_runner PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(unit_test_runner PRIVATE -fsanitize=address)

add_executable(integration_test_runner ${INTEGRATION_TEST_SRCS})
target_link_libraries(integration_test_runner PRIVATE squid_core)

target_compile_options(integration_test_runner PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(integration_test_runner PRIVATE -fsanitize=address)

add_executable(performance_test_runner ${PERF_TEST_SRCS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_libraries(performance_test_runner PRIVATE squid_core_release)
else()
	target_link_libraries(performance_test_runner PRIVATE squid_core)
endif()

target_compile_options(performance_test_runner PRIVATE -O3 -DNDEBUG -fomit-frame-pointer)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
	set_target_properties(performance_test_runner PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
