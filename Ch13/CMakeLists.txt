
#总编译文件，是整个VO工程的CMakeLists.txt
cmake_minimum_required(VERSION 3.1)
project(myslam)

############################################################
# Build Type
############################################################
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF()
MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

############################################################
# C++ flags
############################################################
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE  "-std=c++11 -O3 -fopenmp -pthread")

############################################################
# compile output
############################################################
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)  #指明可执行文件的输出保存位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)   #指明生成的库文件的输出保存位置

# 输出comiile 命令，方便Understand使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

############################################################
# Cmake Moudule Path
############################################################
#为了使用g2o等库,需要配以FindG2O.cmake等文件，列出FindG2O.cmake等文件的位置
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
message(${CMAKE_MODULE_PATH})

############### dependencies(找到涉及的各种库) ######################
# Eigen
include_directories("/usr/include/eigen3")

# OpenCV
find_package(OpenCV 3.1 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# pangolin
find_package(Pangolin REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

# Sophus
find_package(Sophus REQUIRED)
include_directories(${Sophus_INCLUDE_DIRS})

# G2O
find_package(G2O REQUIRED)
include_directories(${G2O_INCLUDE_DIRS})

# glog
find_package(glog REQUIRED)
include_directories(${GLOG_INCLUDE_DIRS})

# gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# gflags
find_package(GFlags 0.6.0 REQUIRED)
#include_directories(${GFLAGS_INCLUDE_DIRS})

# csparse
find_package(CSparse REQUIRED)
include_directories(${CSPARSE_INCLUDE_DIR})

#将后面的一大堆OpenCV,Pangolin....等库文件打包为THIRD_PARTY_LIBS
set(THIRD_PARTY_LIBS
        ${OpenCV_LIBS}
        # ${Sophus_LIBRARIES}
		Sophus::Sophus
        ${Pangolin_LIBRARIES} GL GLU GLEW glut
        g2o_core g2o_stuff g2o_types_sba g2o_solver_csparse g2o_csparse_extension
        ${GTEST_BOTH_LIBRARIES}
		glog::glog

        ${GFLAGS_LIBRARIES}
        pthread
        ${CSPARSE_LIBRARY}
        )

enable_testing() #允许测试

############### source and test ######################
include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(src)
#src是myslam的子目录，所以无需附加参数，add_subdirectory是为构建添加一个子路径src，即去到src目录下找对应的CMakeLists文件进行构建编译
add_subdirectory(entry)
