cmake_minimum_required(VERSION 3.16)
project(LHRDB_Lib_NoFloat LANGUAGES C)

add_library(LHRDB_Lib_NoFloat
   src/lhrdb.c
)

target_include_directories(LHRDB_Lib_NoFloat
   PUBLIC include/
)

