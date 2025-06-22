set(CMAKE_CXX_COMPILER gcc)

# 添加包含目录
include_directories(include)

# 添加库目录
link_directories(lib)

# 添加可执行文件 target
add_executable(use-math ./main.c)

# 链接动态库
target_link_libraries(use-math math)

# set_target_properties(use-math PROPERTIES LINK_FLAGS "-Wl,-rpath,./lib")