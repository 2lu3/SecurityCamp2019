#ifndef B_HPP
#define B_HPP
#include <memory>

class A;

class B
{
public:
    B();
    struct Hoge
    {
        int huga = 0;
    };
    std::unique_ptr<A> a;
};

#endif
