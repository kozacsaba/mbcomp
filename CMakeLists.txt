# Main ########################################################################

cmake_minimum_required(VERSION 3.22)

project(MBComp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/juce)
#add_subdirectory(libs/gtest)

add_subdirectory(src)
#add_subdirectory(tests)
