cmake_minimum_required(VERSION 3.11)
project(miner C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-pthread")
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(miner main.c block.c block.h list.c list.h transaction.c transaction.h siggen.c siggen.h)
target_link_libraries(miner Threads::Threads)