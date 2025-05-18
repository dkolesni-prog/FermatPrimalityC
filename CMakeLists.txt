cmake_minimum_required(VERSION 3.15)
project(fermat_primality_c C)

# we’re writing plain C
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# let pkg-config tell us where GMP lives
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)

# our executable
add_executable(fermat_test
        main.c
        # …any other .c files
)

# include headers and link against the GMP library
target_link_libraries(fermat_test
        PRIVATE
        PkgConfig::GMP
)
