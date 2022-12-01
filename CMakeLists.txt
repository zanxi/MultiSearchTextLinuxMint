cmake_minimum_required(VESRSION 3.5)
project(searchtext)

set(SOURCES
   hw14.c
   function14.c
)

set(HEADERS
   function14.h
)


add_executable(searchtext ${SOURCES}$ ${HEADERS}$)

set_target_properties(
   searchtext PROPERTIES
   cxx_standard 11
   cxx_standard_required ON
)