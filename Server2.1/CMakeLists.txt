# ./CMakeLists.txt，工程总配置文件

# 最低版本要求，必须
cmake_minimum_required(VERSION 2.8)

# 工程名
project(bbl)


#message(STATUS "Target system: ${CMAKE_SYSTEM} ${CMAKE_SYSTEM_PROCESSOR}")

#版本信息
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# 加入一个配置头文件，用于处理 CMake 对源码的设置
#configure_file (
#"${PROJECT_SOURCE_DIR}/config.h.in"
# "${PROJECT_SOURCE_DIR}/config.h"
# )

# 添加include目录,本例中文件在./下,所以直接取${PROJECT_SOURCE_DIR}/
include_directories(
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/lib/third/include/
)

# 添加链接库目录,库查找按照参数顺序从左到右查
link_directories(
	${PROJECT_SOURCE_DIR}/libs/libopencv/lib64
	${PROJECT_SOURCE_DIR}/libs/libopencv/share/OpenCV/3rdparty/lib64/	
)

# 设置生成的库文件目录,这里设置成和库目录相同
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#设置可执行文件目录,同样设成和库目录相同
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置C编译器编译选项
set(CMAKE_C_FLAGS "-O2 -DNDEBUG -Wall -pipe -fmessage-length=0 -ffunction-sections -fdata-sections")
#set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -DDEBUG")
#set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

#设置C编译器 连接选项
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-gc-sections")


# 添加子目录（每个子目录下都必须有自已的CMakeLists.txt文件)
#add_subdirectory(external)
#add_subdirectory(lib)
#add_subdirectory(app)
add_subdirectory(test)

# 设置安装路径
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install")

