# CMake 使用

```
v2
created, 2024-11-23T10:49+08:00
published, 2024-11-23T17:30+08:00
category: c-cpp
tags: cmake
```

[toc]

前置阅读:

1. 「ELF 文件的链接与加载」：[博客园](https://www.cnblogs.com/dutrmp19/p/18280144) | <a href="../../../operating-system/elf-linker-loader/elf-linker-loader.md" target="_blank">GitHub</a>

所有 demo code 在 <a href="./cmake-demo/" target="_blank">./cmake-demo</a>（在 GitHub 浏览本博客，链接才可以打开）

## 总结

CMake 相当于在原生的操作系统上 wrap 了一层

-   Linux 上它 wrap 了基于 makefile 的那套构建系统
-   Windows 上它 wrap 了基于 sln 的那套构建系统

执行命令 `cmake <dir>` 只是根据 `dir/CMakeLists.txt` 生成了不同构建系统的构建规则。  
要继续执行 `cmake --build <dir>` 才会根据 `dir` 下的构建规则开始编译。

常用的使用方式为：

1. 在 src 下 `mkdir build` 文件夹并 `cd build`
2. 在 build 文件下 `cmake ..` 把构建规则生成到 build 文件夹下，可能会在 build 文件夹下生成一个 Makefile
3. `cmake --build .` 开始生成

下面表格中，用 `CC` 表示对应的编译器，比如 `gcc` 或 `clang`

| Makefile 命令 | cmake 函数                      |
| :------------ | :------------------------------ |
| CC -I         | include_directories             |
| CC -L         | link_directories                |
| CC            | add_executable                  |
| ar 构建静态库 | add_library                     |
| CC -shared    | add_library(<dst> SHARED <src>) |

## 编译器搜索头文件

### 标准系统头文件目录

```cpp
#include <stdio.h>
int main() {
    printf("hello world\n");
}
```

我们知道编译前会把 include 头文件复制到 .cpp，但是编译器是怎么知道 `stdio.h` 在哪里呢？

参考 [AI 的回答](https://lxblog.com/qianwen/share?shareId=cea61846-358a-410a-8018-51935e9e1b11)：

-   如果用 `<>` 从**标准系统头文件目录**开始搜索，这些目录通常包括 `/usr/include` 和 `/usr/local/include`
    如果找不到，再考虑其他用户指定的路径（如 -I 选项指定的路径）
-   如果用 `""` 从当前源文件所在的目录开始搜索，
    然后才会搜索标准系统头文件目录和其他用户指定的路径（如 -I 选项指定的路径）。

用 `cpp -v` 命令查看标准系统头文件目录

```
$ cpp -v

...balabalabala

#include "..." search starts here:
#include <...> search starts here:
 /usr/lib/gcc/x86_64-linux-gnu/13/include
 /usr/local/include
 /usr/include/x86_64-linux-gnu
 /usr/include
End of search list.
```

所以写作 `#include "stdio.h"` 也是可以的。

```cpp
#include "stdio.h"

int main()
{
    printf("hello world\n");
    return 0;
}
```

### 自定义搜索路径

编辑环境变量

```bash
export CPATH=/path/to/your/include:$CPATH
export CPLUS_INCLUDE_PATH=/path/to/your/include:$CPLUS_INCLUDE_PATH
```

或者使用 `-I` 参数

## vscode configure cmake

[VS Code: Cmake Extension has no button on sidebar \- Stack Overflow](https://stackoverflow.com/questions/67772508/vs-code-cmake-extension-has-no-button-on-sidebar)

## `add_executable`

`add_executable(call_add.c ./main.c ./add.c)`

```
.
├── CMakeLists.txt
├── add.c
└── main.c
```

```CPP
// main.c
#include <stdio.h>
int add(int, int);
int main()
{
    int x = add(1, 2);
    printf("%d\n", x);
    return 0;
}

//add.c
int add(int x, int y) {
    return x + y;
}
```

```cmake
set(CMAKE_CXX_COMPILER gcc)
add_executable(call_add ./main.c ./add.c)
```

对于 Linux 操作系统，cmake 相当于在 gcc/g++ 上面 wrap 一层

执行 `cmake .` 只会生成构建的文件，比如 Makefile，而不会构建出可执行文件
如果要构建 call_add，还要再执行 `make`

## `-I` : include_directories

相当于使用 `-I` 参数，这样使用 `include "path_to_lib_folder/*.h"` 可以改写为

```
#include "*.h"

`gcc -Ipath_to_lib_folder`
```

相当于：

```
include_directories(path_to_lib_folder)
#include "*.h"
```

对于 Linux 操作系统，cmake 相当于在 gcc/g++ 上面 wrap 一层，include_directories wrap 了 -I 参数

## 静态库构建与使用

### 传统构建方法：gcc + ar

当前目录结构如下：

```
math
├── include
│   └── math.h
└── src
    ├── add.c
    └── sub.c
```

add.c 和 sub.c 都 include math.h，我要如何构建一个静态库 math.a?

要将 `add.c` 和 `sub.c` 文件构建为一个静态库 `math.a`，你需要使用编译器（如 GCC）来编译这些源文件，并使用归档工具（如 `ar`）来创建静态库。以下是具体步骤：

#### 1. 构建 `.o`

将每个 `.c` 文件编译成对应的 `.o`（对象文件）

```sh
gcc -c src/add.c -Iinclude -o src/add.o
gcc -c src/sub.c -Iinclude -o src/sub.o
```

这里 `-c` 参数告诉编译器只进行编译和汇编，而不进行链接，因此会生成 `.o` 文件。`-Iinclude` 参数指定了头文件的搜索路径。

#### 2. 使用 `ar` 工具构建 `.a`

接下来，使用 `ar` 工具将这些对象文件打包成一个静态库 `math.a`：

```sh
ar rcs libmath.a src/add.o src/sub.o
```

这里的 `ar` 命令参数解释如下：

-   `r`：插入成员文件到归档文件中，如果成员文件已经存在，则替换它。
-   `c`：创建新的归档文件。
-   `s`：创建或更新归档文件的索引。

#### 3. 检查静态库

你可以使用 `ar` 或 `nm` 命令来检查静态库的内容：

```sh
ar t libmath.a
nm libmath.a
```

#### 4. 使用静态库

````
use-math
├── include
│   └── math.h
├── lib
│   └── libmath.a
└── main.c

假设你有一个主程序 `main.c`，并且你想链接这个静态库，可以这样做：

```sh
gcc main.c -Llib -lmath -o main -Iinclude
````

-   `-Iinclude` 参数指定头文件搜索目录（当前目录的 include 文件夹内）
-   `-Llib` 参数指定了库文件的搜索路径（当前目录的 lib 文件夹内）
-   `-lmath` 参数告诉链接器链接 `libmath.a` 库，会自动添加 `lib` prefix 并假设后缀名为 `.a`

### cmake 构建静态库：`add_library`

```
.
├── CMakeLists.txt
├── include
│   └── math.h
└── src
    ├── add.c
    └── sub.c
```

```Cmake
include_directories(./include)
add_library(math ./src/add.c ./src/sub.c)
```

执行 `cmake .` 生成 Makefile 后，执行 `make` 就在当前文件夹下打包出了 `libmath.a`

### cmake 链接静态库：`target_link_libraries`

```
.
├── CMakeLists.txt
├── include
│   └── math.h
├── lib
│   └── libmath.a
└── main.c
```

```cmake
## 添加包含目录
include_directories(include)

## 添加库目录
link_directories(lib)

## 添加可执行文件 target
add_executable(use-math ./main.c)

## 链接静态库
target_link_libraries(use-math libmath.a)
```

## 动态库构建与使用

### linux 下动态库存放路径

在 Linux 系统中，动态链接库（`.so`文件）通常存放在以下几个标准目录中：

1. **/lib** 和 **/lib64**：

    - 这些目录通常存放系统核心的动态链接库，特别是那些启动和运行系统所必需的库。例如，`libc.so`（C 标准库）通常会存放在这些目录中。

2. **/usr/lib** 和 **/usr/lib64**：

    - 这些目录存放用户安装的各种软件包的动态链接库。大多数非核心的库都会存放在这些目录中。

3. **/usr/local/lib** 和 **/usr/local/lib64**：

    - 这些目录通常用于存放本地安装的软件包的动态链接库，尤其是那些通过源代码编译安装的软件。

4. **/opt/**：

    - 有些软件包可能会将其动态链接库存放在 `/opt/` 目录下的子目录中，特别是那些独立的或第三方的应用程序。

5. **/usr/lib/x86_64-linux-gnu** 和 **/usr/lib/i386-linux-gnu**：
    - 这些目录是特定于架构的库目录，通常用于存放针对特定架构优化的库文件。例如，`x86_64-linux-gnu` 目录用于存放 64 位架构的库，而 `i386-linux-gnu` 目录用于存放 32 位架构的库。

### gcc 构建动态库 `*.so`

```
├── include
│   └── math.h
└── src
    ├── add.c
    └── sub.c
```

```shell
# 编译源文件为对象文件
gcc -c -fPIC src/add.c -o src/add.o
gcc -c -fPIC src/sub.c -o src/sub.o

# 链接对象文件生成共享库
gcc -shared src/add.o src/sub.o -o libmath.so
```

要使用已经编译好的共享库 `libmath.so`，你需要在编译和运行程序时正确地链接和加载这个库。以下是详细的步骤和命令，帮助你在给定的目录结构中使用 `libmath.so` 文件。

### 使用 gcc 生成依赖动态库的可执行文件

```
.
├── include
│   └── math.h
├── lib
│   └── libmath.so
└── main.c
```

```c
// main.c
#include <stdio.h>
#include "math.h"

int main() {
    int a = 10, b = 5;
    printf("Add: %d + %d = %d\n", a, b, add(a, b));
    printf("Sub: %d - %d = %d\n", a, b, sub(a, b));
    return 0;
}
```

1. **编译 `main.c` 并链接 `libmath.so`**：

    ```sh
    gcc main.c -I./include -L./lib -lmath -o main
    ```

    解释：

    - `-I./include`：指定头文件的搜索路径。
    - `-L./lib`：指定库文件的搜索路径。
    - `-lmath`：链接 `libmath.so` 库。
    - `-o main`：指定输出文件名为 `main`。

2. **运行程序**：
   在运行程序之前，确保动态链接器能够找到 `libmath.so`。你可以通过以下几种方式之一来实现：

    - **临时设置 `LD_LIBRARY_PATH`**：

        ```sh
        export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH
        ./main
        ```

    - **永久设置 `LD_LIBRARY_PATH`**：
      你可以在你的 shell 配置文件（如 `.bashrc` 或 `.zshrc`）中添加以下行：

        ```sh
        export LD_LIBRARY_PATH=/path/to/your/lib:$LD_LIBRARY_PATH
        ```

    - **将 `libmath.so` 复制到系统库目录**：
      如果你有管理员权限，可以将 `libmath.so` 复制到系统的标准库目录（如 `/usr/local/lib`），然后更新动态链接器缓存：
        ```sh
        sudo cp ./lib/libmath.so /usr/local/lib/
        sudo ldconfig
        ```

### cmake 构建动态库：`add_library(<dst> SHARED <src>)`

对于 Linux 下，构建动态库只需要添加 shared 关键字即可

```cmake
include_directories(./include)
add_library(math SHARED ./src/add.c ./src/sub.c)
```

```
math
├── CMakeLists.txt
├── include
│   └── math.h
└── src
    ├── add.c
    └── sub.c
```

执行 `cmake .` 得到 Makefile，然后执行 `make` 得到 `libmath.so` 文件

### cmake 链接动态库：`target_link_libraries`

```
use-math
.
├── CMakeLists.txt
├── include
│   └── math.h
├── lib
│   └── libmath.so
└── main.c
```

```cmake
set(CMAKE_CXX_COMPILER gcc)

## 添加包含目录
include_directories(include)

## 添加库目录
link_directories(lib)

## 添加可执行文件 target
add_executable(use-math ./main.c)

## 链接动态库
target_link_libraries(use-math math) # <-- 只修改了这里，静态链接使用 `libmath.a`，动态链接直接填 `math`

## 最后一行是指定动态链接库查找位置为当前路径下 `./lib`，也可以采用编辑 LD_LIBRARY_PATH 的方法替代
## set_target_properties(use-math PROPERTIES LINK_FLAGS "-Wl,-rpath,./lib")
```

可能是由于 cmake 版本的问题，我电脑上动态链接不需要修改 `LD_LIBRARY_PATH`，也不需要用 `-Wl,-rpath`，
可能是 cmake 构建的时候，悄悄添加了 `set_target_properties`
用 `ldd <exe>` 就可以查看动态链接库依赖路径

```
> /use-math$ ldd ./use-math
        linux-vdso.so.1 (0x00007ffcbf59f000)
        libmath.so => lib/libmath.so (0x00007f42bdd42000) # <--- look here, cmake builds with this link
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f42bdb2a000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f42bdd4e000)
```

> ldd 是通过调用动态链接器来解析和显示可执行文件或共享库的依赖关系。
> 它不是一个简单的文件内容读取工具，而是利用动态链接器的功能来生成依赖库列表。
> 这种方式确保了 ldd 的输出反映了实际运行时的依赖关系。

如果此时把 lib/libmath.so 移动到 libxxx/libmath.so 下，就无法执行 use-math，

### Windows 要用 `__declspec`

Windows 对于动态库的导入和导出有自己的写法：
`__declspec(dllexport)`
`__declspec(dllimport)`

参考:

-   https://lxblog.com/qianwen/share?shareId=81bcd5e5-fc7c-4995-a4df-88441854f432
-   https://www.bilibili.com/video/BV1Qu4y177zz?t=1320

Windows 下构架动态库会得到 lib 和 dll 文件
windows 下链接只需要 lib 文件，运行时候加载才需要 dll

对于 Windows cmake 相当于在基于 `sln` 的构建方案上 wrap 了一层

## cmake 定义变量

```shell
cmake -DDEBUG=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local..
```

定义了

1. DEBUG 这个变量的值为 ON
2. CMAKE_BUILD_TYPE 这个变量的值为 Debug

### 特殊的变量

|        变量         |                    作用                     |   例子    |
| :-----------------: | :-----------------------------------------: | :-------: |
| `CMAKE_PREFIX_PATH` | find_package 会在该变量指定的路径中查找依赖 | dir1;dir2 |

## 使用第三方库：`find_package`

第三方库可能要提供 `lib`，`bin` 和 `include`。
如果第三方库支持通过 cmake 来导入， 它要提供几个 `.cmake` 文件。

find_package 有两种 search mode[^1]:

1. module mode
2. config mode：第三方库需要提供形如 `package-config.cmake` 或者驼峰命名法的文件，会在很多可能的路径下搜索[^2]，比如：
    1. 环境变量的 `<PackageName>_DIR`
    2. `CMAKE_PREFIX_PATH`
    3. `CMAKE_FRAMEWORK_PATH`
    4. `CMAKE_APPBUNDLE_PATH`

参考：[【CMake 第六讲】：find_package 收尾，去享受参透一切的喜悦吧~\_哔哩哔哩\_bilibili](https://www.bilibili.com/video/BV1sc41167hj/)

从系统环境变量里找到另一个第三方库的的 `.cmake` 文件，然后执行，得到相应的动态库的路径作为变量，方便后续的执行流使用该变量。

grpc 的安装教程里[^3]，就会引导用户在 `~/.local/` 下安装第三方库，  
使用时候需要把 `path to cmake/package` 添加到 `CMAKE_PREFIX_PATH` 中[^4]。

```shell
$ pwd
~/.local/lib/cmake

$ tree .
.
├── absl
│   ├── abslConfig.cmake
│   ├── abslConfigVersion.cmake
│   ├── abslTargets-noconfig.cmake
│   └── abslTargets.cmake
├── c-ares
│   ├── c-ares-config-version.cmake
│   ├── c-ares-config.cmake
│   ├── c-ares-targets-noconfig.cmake
│   └── c-ares-targets.cmake
├── grpc
│   ├── gRPCConfig.cmake
│   ├── gRPCConfigVersion.cmake
│   ├── gRPCPluginTargets-noconfig.cmake
│   ├── gRPCPluginTargets.cmake
│   ├── gRPCTargets-noconfig.cmake
│   ├── gRPCTargets.cmake
│   └── modules
│       ├── Findc-ares.cmake
│       ├── Findre2.cmake
│       └── Findsystemd.cmake
...
```

## 参考

[^1]: [find_package — CMake 3\.31\.3 search procedure](https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure)
[^2]: [find_package — CMake 3\.31\.3 Documentation](https://cmake.org/cmake/help/latest/command/find_package.html#id10)
[^3]: [Quick start \| C\+\+ \| gRPC](https://grpc.io/docs/languages/cpp/quickstart/#install-grpc)
[^4]: [Using Dependencies Guide — CMake 3\.31\.3 Documentation](https://cmake.org/cmake/help/latest/guide/using-dependencies/index.html#config-file-packages)
