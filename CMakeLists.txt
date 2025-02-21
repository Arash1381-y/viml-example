cmake_minimum_required(VERSION 3.14)
project(example VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        flexisim
        QUIET
        GIT_REPOSITORY git@github.com:Arash1381-y/undefined.git
        GIT_TAG main
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/ext/flexisim
)

FetchContent_MakeAvailable(flexisim)


add_executable(first_test src/main.cpp)
target_link_libraries(first_test flexisim)

set_target_properties(first_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
