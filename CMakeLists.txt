cmake_minimum_required(VERSION 3.20)
include(CheckCXXSourceRuns)
project(PiKtures VERSION 1.0.0.2)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
option(BUILD_TESTS "Build unit testing sources" OFF)
option(USE_OPENCV_GUI "Use OpenCV high-level GUI" ON)
find_package(OpenCV REQUIRED)
set(CMAKE_REQUIRED_INCLUDES ${OpenCV_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${OpenCV_LIBS})
if(${USE_OPENCV_GUI})
    check_cxx_source_runs(
        "#include<opencv2/opencv.hpp>
        #include<iostream>
        using namespace cv;
        using namespace std;
        int main(){
            Mat test_mat = Mat::ones(cv::Size(1, 1), CV_8U);
            try{
                imshow(\"check_imshow\", test_mat);
            }catch(exception& e){
                return 1;
            }
            return 0;
        }"
        IMSHOW_AVAILABLE
    )
    if(NOT ${IMSHOW_AVAILABLE})
        message(WARNING "OpenCV high-level GUI required but not available.")
        set(USE_OPENCV_GUI false)
    endif()
endif()
if(${USE_OPENCV_GUI})
    set(USE_OPENCV_GUI true)
endif()
configure_file(includes/PiKturesConfig.hpp.in PiKturesConfig.hpp)
include_directories(${OpenCV_INCLUDE_DIRS} includes)
add_subdirectory(modules)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
add_executable(${PROJECT_NAME} PiKtures.cpp)
target_link_libraries(${PROJECT_NAME} utility rowdy detector command enhancer)
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}")
