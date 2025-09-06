// C -> A
// C -> B
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