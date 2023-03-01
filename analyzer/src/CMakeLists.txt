cmake_minimum_required(VERSION 3.5.1)
project(KANALYZER)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++14 support
# for your compiler.
# Check for C++14 support and set the compilation flag
include(CheckCXXCompilerFlag)

add_compile_options(-std=c++20 -fPIC -Wall -march=native -pipe -flto=8)
add_compile_options(-fuse-linker-plugin -ffunction-sections -fdata-sections)
add_compile_options(-fgraphite-identity -floop-nest-optimize -fipa-pta)
add_compile_options(-fdevirtualize-at-ltrans -fno-semantic-interposition)

link_libraries(-std=c++20 -fPIC -Wall -march=native -pipe -flto=8)
link_libraries(-fuse-linker-plugin -ffunction-sections -fdata-sections)
link_libraries(-fgraphite-identity -floop-nest-optimize -fipa-pta)
link_libraries(-fdevirtualize-at-ltrans -fno-semantic-interposition)
link_libraries(-fuse-ld=mold -Wl,-O1 -Wl,-as-needed -Wl,--gc-sections)
#CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
# if(COMPILER_SUPPORTS_CXX14)
# 	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-rtti -fPIC -Wall")
# else()
# 	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
# endif()

include_directories(${LLVM_INCLUDE_DIRS})
message(STATUS "LLVM include path: ${LLVM_INCLUDE_DIRS}")
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory (lib)
