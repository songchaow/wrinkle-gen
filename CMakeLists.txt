CMAKE_MINIMUM_REQUIRED ( VERSION 3.1.0 )

project(wrinkle-gen)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB WRINKLE_GEN_SRC
    *.hpp
    *.h
    *.cpp
    ext/*
)

# test
file(GLOB TEST_ENTRY test/*)
add_executable(gen-test ${WRINKLE_GEN_SRC} ${TEST_ENTRY})