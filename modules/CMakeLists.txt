cmake_minimum_required(VERSION 3.20)
add_library(rowdy rowdy.cpp)
target_link_libraries(rowdy ${OpenCV_LIBS})
