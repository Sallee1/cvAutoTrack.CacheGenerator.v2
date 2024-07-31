cmake_minimum_required(VERSION 3.10)

project(pip-remake)

find_package(fmt CONFIG REQUIRED)

add_executable(pip-remake main.cpp)

target_compile_options(pip-remake PRIVATE /std:c++latest)

if(VCPKG_TARGET_TRIPLET MATCHES "static")
    target_compile_options(pip-remake PRIVATE /MT$<$<CONFIG:Debug>:d>)
else()
    target_compile_options(pip-remake PRIVATE /MD$<$<CONFIG:Debug>:d>)
endif()

target_link_libraries(pip-remake PRIVATE fmt::fmt)