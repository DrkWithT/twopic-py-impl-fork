cmake_minimum_required(VERSION 3.20)

project(LexicalAnalyzer VERSION 1.0 LANGUAGES CXX)

find_package(fmt REQUIRED)

if(DEFINED SANITIZER_FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
    message(STATUS "Sanitizers enabled: AddressSanitizer, UndefinedBehaviorSanitizer")
endif()

add_library(frontend)
target_include_directories(frontend PUBLIC ${PROJECT_SRC_DIR})
target_sources(frontend PRIVATE ${PROJECT_SRC_DIR}/frontend/lexical.cpp ${PROJECT_SRC_DIR}/frontend/parser.cpp)

add_executable(lexical_analyzer)
target_include_directories(lexical_analyzer PUBLIC ${PROJECT_SRC_DIR})
target_sources(lexical_analyzer PRIVATE ${PROJECT_SRC_DIR}/main.cpp)

# For working around a regression within my Homebrew LLVM 21.1.7 installs under Clang: (issue 235411) - DerkT
if (APPLE AND LLVM_LIBRARY_DIR)
    target_link_directories(lexical_analyzer PUBLIC "${LLVM_LIBRARY_DIR}/c++" PUBLIC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()

target_link_libraries(lexical_analyzer PRIVATE frontend fmt::fmt)

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

