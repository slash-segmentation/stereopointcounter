cmake_minimum_required(VERSION 2.8)
 
project(stereopointcounter)
 
# Get the correct c++XX flag
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set (CMAKE_CXX_FLAGS " -std=c++11 ${CMAKE_CXX_FLAGS}")
elseif(COMPILER_SUPPORTS_CXX0X)
	set (CMAKE_CXX_FLAGS " -std=c++0x ${CMAKE_CXX_FLAGS}")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_library(StereoLib STATIC src/StereoPointCount.cpp
               src/ImageUtils.hpp src/StereoPointCount.hpp)

add_executable(stereopointcounter MACOSX_BUNDLE src/main.cpp src/optionparser.h)
target_link_libraries(stereopointcounter StereoLib ${ITK_LIBRARIES} )
