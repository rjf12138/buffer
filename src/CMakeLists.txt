project(buffer)

set(LIBRARY_OUTPUT_PATH /home/ruanjian/workspace/project_pre/buffer/output/debug/lib)

aux_source_directory(. DIR_LIB_SRCS)

add_library(buffer ${DIR_LIB_SRCS})

target_link_libraries(buffer -lpthread)
target_link_libraries(buffer -lsystem_util)

