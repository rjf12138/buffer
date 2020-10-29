
project(buffer)

set(EXECUTABLE_OUTPUT_PATH /home/ruanjian/workspace/project_pre/buffer/output/debug/bin)

add_executable(ByteBuffer_Test ByteBuffer_Test.cc)

target_link_libraries(ByteBuffer_Test -lbuffer)
target_link_libraries(ByteBuffer_Test -lgmock)
target_link_libraries(ByteBuffer_Test -lgmock_main)
target_link_libraries(ByteBuffer_Test -lgtest)
target_link_libraries(ByteBuffer_Test -lgtest_main)
target_link_libraries(ByteBuffer_Test -lpthread)
target_link_libraries(ByteBuffer_Test -lsystem_util)

