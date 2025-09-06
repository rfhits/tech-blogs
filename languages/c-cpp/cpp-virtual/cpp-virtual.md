# 虚函数、虚表

-   created: 2024-06-28T10:55+08:00
-   modified: 2024-11-17T16:05+08:00
-   published: 2024-11-17T16:18+08:00
-   category: C-CPP

## `virtual` 关键字的作用

多态分为静态多态和运行时多态。

C++ 有一点和 Java 不一样，如果没有 `virtual` 关键字，是没法实现运行时多态的。

### 父类指针指向子类对象调用虚函数会调用子类重写版本

父类指针指向子类对象时候，指针调用 virtual 关键字修饰的函数，会调用子类的重写（override）版本。

例子：对于 Shape 父类，子类有 Square、Circle，定义 `virtual float area() const;`。
可以对 `vector<Shape*> shapes` 迭代获取所有图形的面积，下面的例子在 [./shape-area.cpp](./shape-area.cpp)

```cpp
#include <iostream>
#include <vector>
#include <memory>
using std::make_shared;
using std::shared_ptr;
using std::vector;

class Shape
{
public:
    virtual double area() const = 0; // pure virtual function, have to be implemented in derived classes

    // if use this function, none of the derived classes will be able to override
    // double area()
    // {
    //     std::cout << "Shape::area()" << std::endl;
    //     return 0;
    // }
    ~Shape() {};
};

class Circle : public Shape
{
public:
    double r_;
    Circle(double r) : r_(r) {}

    double area() const { return 3.14 * r_ * r_; }
    ~Circle() { std::cout << "~Circle()" << std::endl; }
};

class Square : public Shape
{
public:
    double s_; // side length of square
    Square(double s) : s_(s) {}
    double area() const { return s_ * s_; }
    ~Square() { std::cout << "~Square()" << std::endl; }
};

int main()
{
    vector<shared_ptr<Shape>> shapes;
    shapes.push_back(make_shared<Circle>(10));
    shapes.push_back(make_shared<Square>(10));
    for (const auto &shape : shapes)
    {
        std::cout << shape->area() << std::endl;
    }
}
```

### `typeid(BaseClass*) -> DerivedClass::typeinfo`

[The typeid operator \(C\+\+ only\) \- IBM Documentation](https://www.ibm.com/docs/en/zos/2.4.0?topic=expressions-typeid-operator-c-only)

IBM 上提供了 `typeid` 的用例，该运算符返回一个 `type_info` 的实例。

同样是继承 `B -> A`， `D -> C`，只差了一个 virtual 关键字。
如果没有 virtual，那么提供给 `typeid(ptr)` 指向什么类型，它就返回什么类型。

```cpp
#include <iostream>
#include <typeinfo>
using namespace std;

struct A
{
    virtual ~A() {} // "virtual" keyword
};

struct B : A
{
};

struct C
{
};
struct D : C
{
};

int main()
{
    B b_obj;
    A *ap = &b_obj;
    A &ar = b_obj;
    cout << "ap: " << typeid(*ap).name() << endl; // B
    cout << "ar: " << typeid(ar).name() << endl;  // B

    D d_obj;
    C *cp = &d_obj;
    C &cr = d_obj;
    cout << "cp: " << typeid(*cp).name() << endl; // C
    cout << "cr: " << typeid(cr).name() << endl;  // C
}
```

## 虚表

### 继承带来的两个指令层面的问题：`this` 的调整和向下转型

1. 使用父类指针调用重写的子类函数的时候，函数传入的 this 指针应该指向子类，所以 this 的位置可能需要调整
    ```cpp
    // 假设 C-> A, C-> B，是一个多继承的结构
    // A, B, C 都实现了 f()
    A* p_a = new C();
    B* p_b = new C();
    p_a->f(); // C::f() 被调用，调用时候 this 需要保证指向一个 C 类实例，p_a 不一定指向 C 类实例
    p_b->f(); // C::f() 被调用，调用时候 this 需要保证指向一个 C 类实例，p_b 不一定指向 C 类实例
    ```
2. 父类指针转换为子类的时候，指针的位置可能需要调整:
    ```cpp
    // 假设 C-> A, C-> B，是一个多继承的结构, class C: public A, public B
    A* p_a = new C();
    B* p_b = new C();
    B* p_real_b = new B();
    C* p_c = dynamic_cast<C*>(p_b); // cast success
    C* p_c_fail = dynamic_cast<C*>(p_real_b); // cast failed, p_c_failed = nullptr
    ```

参考代码，见 [./multi-inherit.cpp](./multi-inherit.cpp)：

```cpp
//  A   B
//   \ /
//    C
#include <iostream>
class A
{
public:
    virtual void f() { std::cout << "A::f()" << std::endl; }
};

class B
{
public:
    char b_ = 'b';

public:
    virtual void f() { std::cout << "B::f()" << std::endl; }
};

class C : public A, public B
{
public:
    void f() { std::cout << "C::f()" << std::endl; }
};

int main()
{
    A *p_a = new C();
    B *p_b = new C();
    p_a->f();
    p_b->f();

    C *p_c = dynamic_cast<C *>(p_b);
    if (p_c)
    {
        std::cout << "dynamic_cast success" << std::endl;
    }

    B *p_real_b = new B();
    C *p_cast_failed = dynamic_cast<C *>(p_real_b);
    if (p_cast_failed == nullptr)
    {
        std::cout << "dynamic_cast failed" << std::endl;
    }
    return 0;
}
```

### 单继承 与 RTTI

> In computer programming, run-time type information or run-time type identification (RTTI) is a feature of some programming languages (such as C++, Object Pascal, and Ada) that exposes information about an object's data type at runtime.
> [Run\-time type information \- Wikipedia](https://en.wikipedia.org/wiki/Run-time_type_information)

提供一个单继承的例子，来自[^1]

```cpp
struct A
{
    int ax;
    virtual void f0() {}
};

struct B : public A
{
    int bx;
    virtual void f1() {}
};

struct C : public B
{
    int cx;
    void f0() override {}
    virtual void f2() {}
};
```

内存布局如下：

```
                                                    C VTable（不完整）
                                                    +------------+
object                                              | RTTI for C |
    0 - struct A, B, C, ptr_C       --------------> +------------+
    8 -     int ax                                  |   C::f0()  |
    12 -   int bx                                   +------------+
    16 - int cx                                     |   B::f1()  |
                                                    +------------+
                                                    |   C::f2()  |
sizeof(C): 24    align: 8                           +------------+
```

单继承对于要解决的两个问题的方案是：

1. 父类指针调用子类方法，this 不需要调整了，实例的内存布局中，大家都在最上面。
2. 父类向下转型总是可以成功，不需要调整指针位置。

### 多继承下，primary base 在头部，其他父类在尾部

```
A   B
 \ /
  C
```

加入有多继承，第一个继承的类 A 就是 C 的 primary base

1. `thunk` 用来解决父类指针调用子类函数时候，this 的调整
2. `offset_to_top` 用来解决父类向下转型对 this 的调整

以下例子还是来自[^1]:

```
                                                C Vtable (7 entities)
                                                +--------------------+
struct C                                        | offset_to_top (0)  |
object                                          +--------------------+
    0 - struct A (primary base)                 |     RTTI for C     |
    0 -   vptr_A -----------------------------> +--------------------+
    8 -   int ax                                |       C::f0()      |
   16 - struct B                                +--------------------+
   16 -   vptr_B ----------------------+        |       C::f1()      |
   24 -   int bx                       |        +--------------------+
   28 - int cx                         |        | offset_to_top (-16)|
sizeof(C): 32    align: 8              |        +--------------------+
                                       |        |     RTTI for C     |
                                       +------> +--------------------+
                                                |    Thunk C::f1()   |
                                                +--------------------+

```

thunk 要调整的偏移不一定等于 offset_to_top，
因为可能还有其他的继承情况，如下：

```
   B.f
   |
A  C.f
 \ /
  D
```

B 中的 thunk 会到 C，而 B 的 offset_to_top 会到 A。

下面这个例子展示了多继承情况下，父类指针指向的对象相同（都是 C），但是指针的地址却不同：

```cpp
struct A
{
    int a{0};
    virtual ~A() {} // a virtual keyword
};

struct B
{
    int b{1};
    virtual ~B() {}
};

struct C : public A, B
{
    int c{2};
};

int main()
{
    C c_obj = *new C();
    C *c_ptr = &c_obj;
    A *a_ptr = &c_obj;
    B *b_ptr = &c_obj;
    std::cout << "c ptr:" << c_ptr << std::endl; // 0x2b0f3ffda0
    std::cout << "a ptr:" << a_ptr << std::endl; // 0x2b0f3ffda0 // base class, same address
    std::cout << "b ptr:" << b_ptr << std::endl; // 0x2b0f3ffdac // there is an offset
    return 0;
}
```

### 虚继承下，virtual class 在实例的尾部

例子还是来自 [^1]:

```
struct A
{
    int ax;
    virtual void f0() {}
    virtual void bar() {}
};

struct B : virtual public A           /****************************/
{                                     /*                          */
    int bx;                           /*             A            */
    void f0() override {}             /*           v/ \v          */
};                                    /*           /   \          */
                                      /*          B     C         */
struct C : virtual public A           /*           \   /          */
{                                     /*            \ /           */
    int cx;                           /*             D            */
    void f0() override {}             /*                          */
};                                    /****************************/

struct D : public B, public C
{
    int dx;
    void f0() override {}
};

```

B 的内存模型，A 作为 virtual base，会放到尾部。
所以当向上转型到 A 的时候，不能再使用 offset_to_top 了，而是要使用 vbase_offset。

vcall_offset 和 thunk 是对问题一「父类指针调用子类函数需要调整 this」的另一种解决方法，
将 thunk 需要调整的偏移记录在虚表中，而不是直接跳转到一段独立的代码去调整 this 再 call 子类的方法。

```
                                          B VTable
                                          +---------------------+
                                          |   vbase_offset(16)  |
                                          +---------------------+
                                          |   offset_to_top(0)  |
struct B                                  +---------------------+
object                                    |      RTTI for B     |
    0 - vptr_B -------------------------> +---------------------+
    8 - int bx                            |       B::f0()       |
   16 - struct A                          +---------------------+
   16 -   vptr_A --------------+          |   vcall_offset(0)   |x--------+
   24 -   int ax               |          +---------------------+         |
                               |          |   vcall_offset(-16) |o----+   |
                               |          +---------------------+     |   |
                               |          |  offset_to_top(-16) |     |   |
                               |          +---------------------+     |   |
                               |          |      RTTI for B     |     |   |
                               +--------> +---------------------+     |   |
                                          |     Thunk B::f0()   |o----+   |
                                          +---------------------+         |
                                          |       A::bar()      |x--------+
                                          +---------------------+
```

D 的内存模型

```
                                          D VTable
                                          +---------------------+
                                          |   vbase_offset(32)  |
                                          +---------------------+
struct D                                  |   offset_to_top(0)  |
object                                    +---------------------+
    0 - struct B (primary base)           |      RTTI for D     |
    0 -   vptr_B  ----------------------> +---------------------+
    8 -   int bx                          |       D::f0()       |
   16 - struct C                          +---------------------+
   16 -   vptr_C  ------------------+     |   vbase_offset(16)  |
   24 -   int cx                    |     +---------------------+
   28 - int dx                      |     |  offset_to_top(-16) |
   32 - struct A (virtual base)     |     +---------------------+
   32 -   vptr_A --------------+    |     |      RTTI for D     |
   40 -   int ax               |    +---> +---------------------+
sizeof(D): 48    align: 8      |          |       D::f0()       |
                               |          +---------------------+
                               |          |   vcall_offset(0)   |x--------+
                               |          +---------------------+         |
                               |          |   vcall_offset(-32) |o----+   |
                               |          +---------------------+     |   |
                               |          |  offset_to_top(-32) |     |   |
                               |          +---------------------+     |   |
                               |          |      RTTI for D     |     |   |
                               +--------> +---------------------+     |   |
                                          |     Thunk D::f0()   |o----+   |
                                          +---------------------+         |
                                          |       A::bar()      |x--------+
                                          +---------------------+
```

为什么不全都使用 non-virtual Thunk 呢？StackOverflow 上说是为了减少 Cache Miss [c\+\+ \- Why are virtual thunks necessary? \- Stack Overflow](https://stackoverflow.com/questions/44397573/why-are-virtual-thunks-necessary)

## 总结

虚表要解决继承带来的两个问题：

1. 使用父类指针调用重写的子类函数的时候，需要调整 this
2. 向下转型

解决方法：

1. 将类的内存布局是嵌套，子类包着父类，构造的时候从内而外，析构的时候从外到内
2. 因为内存布局是嵌套的，指针会依据自己的类型调整指向对象的具体位置，以保证在访问对象成员时候的偏移一致
3. 实例内部会有一个 vptr 专门指向虚表，多继承下的 Derived 实例内部会有多个 vptr 指向不同基类的虚表，如 `D->P1, D->P2`，D 内部会有两根 vptr
4. 多继承下，虚表内部需要
    1. offset_to_top：从子类到基类的偏移，用于从子类访问父类成员调整 this 指针、用于子类父类类型转换的时候调整指针
    2. non-virtual Thunk: 调用函数的时候调整 this
5. 虚继承下，虚表内部需要：
    1. offset_to_top
    2. virtual Thunk 和 vcall_offset：不再将 this 偏移的调整放到 Thunk 中
    3. vbase_offset: 此时内存布局下，基类不在最顶上，所以需要各个子类到基类的偏移

## 面试问题

#### 实例考查

[👨‍💻 量化相关 \- 咸鱼暄的代码空间](https://xuan-insr.github.io/interviews/quant/#session-1)

```CPP
struct A {
    int a = 0;
    char b;
    long long c = 1;
    virtual void printa() = 0;
    virtual void printb() { cout << b; }
    void printc() { cout << c; }
};

struct B : public A {
    void printa() { cout << 1; }
    void printb() { cout << 2; }
    void printc() { cout << 3; }
};

void print();

int main() {
    A* bar = new B();
}
```

问 bar->printa(); 然后 bar->printb(); 然后 bar->printc();，让对比这三次访问的速度。第一次最慢，因为要先找 vtable 再找函数地址。第二次稍快，因为 vtable 会在 cache 里。第三次最快，因为静态绑定使得函数地址就在汇编里。

`sizeof(A)` 在 64 位下，为 24，查看 `clang++ -cc1 -emit-llvm -fdump-record-layouts -fdump-vtable-layouts test.cpp`。
因为 vptr 占据了 8 个 bytes。

```
*** Dumping AST Record Layout
         0 | struct A
         0 |   (A vtable pointer)
         8 |   int a
        12 |   char b
        16 |   long long c
           | [sizeof=24, dsize=24, align=8,
           |  nvsize=24, nvalign=8]
```

#### 虚表什么时候创建

编译时期构建的。

> “表格中的 virtual functions 地址是如何被建构起来的？在 C++ 中，virtual functions（可经由其 class object 被调用）可以在编译时期获知。此外，这一组地址是固定不变的，执行期不可能新增或替换之。由于程序执行时，表格的大小和内容都不会改变，所以其建构和存取皆可以由编译器完全掌控，不需要执行期的任何介入。”
> [C\+\+中的虚函数表是什么时期建立的？ \- 峰子\_仰望阳光 \- 博客园](https://www.cnblogs.com/xiehongfeng100/p/4678892.html)

C++ 类的虚函数表和虚函数在内存中的位置
虚函数表指针是虚函数表所在位置的地址。虚函数表指针属于对象实例。因而通过 new 出来的对象的虚函数表指针位于堆，声名对象的虚函数表指针位于栈
虚函数表位于只读数据段（.rodata），即：C++ 内存模型中的常量区；
虚函数代码则位于代码段（.text），也就是 C++ 内存模型中的代码区

[C\+\+类的虚函数表和虚函数在内存中的位置\-CSDN 博客](https://blog.csdn.net/JMW1407/article/details/108243316)

#### C++ virtual 函数可以被标记为 inline 吗

虚函数可以是内联函数，内联是可以修饰虚函数的，但是当虚函数表现多态性的时候不能内联。
内联是在编译期**建议**编译器内联，而虚函数的多态性在运行期，编译器无法知道运行期调用哪个代码，因此虚函数表现为多态性时（运行期）不可以内联。
inline virtual 唯一可以内联的时候是：编译器知道所调用的对象是哪个类（如 Base::who()），这只有在编译器具有实际对象而不是对象的指针或引用时才会发生。

#### virtual 和 构造函数、析构函数的关系

构造函数不能被标记为 virtual。IDE 也会报红的。
virtual 意思是，根据对象的真实类型调用函数，但是在对象没有被构造出来前，其没有任何类型。

析构函数建议标记为 virtual

可以在构造函数和析构函数内调用 virtual 函数，但是调用的函数是当前类型的。
因为构造过程和析构过程会修改虚表内容。
比如 Derived -> Base，在 Base 的构造函数中调用虚函数，此时 Derived 的虚表还未被建立，vtable 中存放的还是 Base 中的函数地址 。
Derived 构造函数中，Derived 虚表已经建立好了，所以此时调用的是自己函数。

```CPP

class Base
{
public:
    Base()
    {
        std::cout << "Base ctor()" << std::endl;
        print();
    };

    virtual void print()
    {
        std::cout << "print in Base" << std::endl;
    }
    virtual ~Base()
    {
        std::cout << "Base dtor()" << std::endl;
        print();
    }
};

class Derived : public Base
{
public:
    void print() {
        std::cout << "print in Derived" << std::endl;
    }

    Derived()
    {
        std::cout << "Derived ctor()" << std::endl;
        print();
    }
    ~Derived()
    {
        std::cout << "Derived dtor()" << std::endl;
        print();
    }
};

int main()
{
    Base *ptr = new Derived();
    delete ptr;
    return 0;
}

// output:
// Base ctor()
// print in Base
// Derived ctor()
// print in Derived
// Derived dtor()
// print in Derived
// Base dtor()
// print in Base
```

[^1]: [C\+\+中虚函数、虚继承内存模型 \- 知乎](https://zhuanlan.zhihu.com/p/41309205)
