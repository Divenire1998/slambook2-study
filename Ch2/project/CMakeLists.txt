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
# 添加可执行文件
add_executable(HelloSlam useHello.cpp)
target_link_libraries(HelloSlam   ${HELLO_LIBRARY})
message("include dir:${HELLO_INCLUDE_DIR}  lib dir:${HELLO_LIBRARY} ")