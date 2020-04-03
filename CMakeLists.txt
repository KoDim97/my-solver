cmake_minimum_required(VERSION 3.15)
project(vector)

set(CMAKE_CXX_STANDARD 14)

add_executable(vector main.cpp interfaces/ICompact.h interfaces/IVector.h interfaces/ISet.h interfaces/ILogger.h interfaces/RC.h
        Vector_Impl.cpp Logger_Impl.cpp Set_Impl.cpp)