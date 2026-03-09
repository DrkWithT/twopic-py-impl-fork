cmake_minimum_required(VERSION 3.20)

project(LexicalAnalyzer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)

if(DEFINED SANITIZER_FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
    message(STATUS "Sanitizers enabled: AddressSanitizer, UndefinedBehaviorSanitizer")
endif()

set(PROJECT_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

add_library(frontend)
target_include_directories(frontend PUBLIC ${PROJECT_SRC_DIR})
target_sources(frontend PUBLIC ${PROJECT_SRC_DIR}/frontend/lexical.cpp ${PROJECT_SRC_DIR}/frontend/parser.cpp ${PROJECT_SRC_DIR}/frontend/ast.hpp)
target_sources(frontend PUBLIC ${PROJECT_SRC_DIR}/print/ast_tree.hpp)
target_link_libraries(frontend PUBLIC fmt::fmt)

add_executable(twopy)
target_include_directories(twopy PUBLIC ${PROJECT_SRC_DIR})
target_sources(twopy PRIVATE ${PROJECT_SRC_DIR}/main.cpp ${PROJECT_SRC_DIR}/backend/bytecode.cpp ${PROJECT_SRC_DIR}/backend/vm.cpp)

# For working around a regression within my Homebrew LLVM 21.1.7 installs under Clang: (issue 235411) - DerkT
if (APPLE AND LLVM_LIBRARY_DIR)
    target_link_directories(twopy PUBLIC "${LLVM_LIBRARY_DIR}/c++" PUBLIC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()

target_link_libraries(twopy PRIVATE frontend fmt::fmt)

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

