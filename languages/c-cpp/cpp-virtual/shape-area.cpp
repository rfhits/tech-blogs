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
    ~Shape() { std::cout << "~Shape()" << std::endl; };
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