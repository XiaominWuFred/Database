cmake_minimum_required(VERSION 3.0)

project(asm2 CXX)
find_package (Threads)


add_executable(ptest txn/txn_processor.cc txn/txn_processor_test.cc txn/lock_manager.cc txn/storage.cc txn/mvcc_storage.cc txn/txn.cc)
target_include_directories(ptest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (ptest ${CMAKE_THREAD_LIBS_INIT})

add_executable(ltest txn/lock_manager.cc txn/lock_manager_test.cc)
target_include_directories(ltest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (ltest ${CMAKE_THREAD_LIBS_INIT})