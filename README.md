# PiKtures
PiKtures is a interactive command-line digital image processing software based on [OpenCV](https://opencv.org), initially developed as the course project of *Signals and Systems*.
## building
### dependence
To build PiKtures, you will need:
- A working C++ compiler supporting C++20 standard features
- CMake >= 3.20
- OpenCV >= 4.5
### express build
Like most software using CMake, just run
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
And you will have a executable built in directory `build`.
### available options
- `BUILD_TESTS`: Build unit testing sources. default: **OFF**.
- `USE_OPENCV_GUI`: Use OpenCV high-level GUI. If this option is disabled or OpenCV high-level GUI is not available, no window will be created and will be replace with file operations. default: **ON**.
