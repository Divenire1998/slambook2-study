############################################################
# 输出可执行文件目录
############################################################
# 二进制文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# 设定库文件编译路径
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

############################################################
# 生成执行文件并链接
############################################################
add_executable(eigenDataType eigenDataType.cpp)

add_executable(eigenSolveMatrix eigenSolveMatrix.cpp)

add_executable(eigenGeometry eigenGeometry.cpp)

add_executable(coordinateTransform coordinateTransform.cpp)

add_executable(plotTrajectory plotTrajectory.cpp)
target_link_libraries( plotTrajectory ${Pangolin_LIBRARIES})

add_executable( visualizeGeometry visualizeGeometry.cpp )
target_link_libraries( visualizeGeometry ${Pangolin_LIBRARIES} )