cmake_minimum_required(VERSION 3.9)
project(cpp-httpd)
link_libraries(boost_system boost_context boost_coroutine boost_thread pthread)
add_executable(cpp-httpd main.cpp)
