cmake_minimum_required(VERSION 2.8)
# set your preferite compiler
#set( CMAKE_C_COMPILER <your-path-to-cc-compiler> )
#set( CMAKE_CXX_COMPILER <your-path-to-c++-compiler> )

project( cwisardrp )
# Enable OpenMP support
if( USE_OMP )
find_package(OpenMP)
if (OPENMP_FOUND)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -DOMP")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DOMP")
else()
message(Building\ without\ OpenMP\ support!!!)
endif()
endif()
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

find_package( OpenCV REQUIRED )
add_executable( cwisardrp main.cpp colorproc.cpp wisard.cpp wnet_lib.cpp pixel_lib.cpp cv_util.cpp comparator.cpp )
target_link_libraries( cwisardrp ${OpenCV_LIBS} )
