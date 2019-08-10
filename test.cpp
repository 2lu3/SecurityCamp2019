// AFile.hpp
#include <memory>
// #include "BFile.hpp"
class B;
struct B::Hoge;

class A
{
    int getHoge(std::unique_ptr<B::Hoge> hoge);
};

// BFile.hpp
// #include "Afile.hpp"

class A;

class B
{
public:
    struct Hoge
    {
        int huga;
    };
    std::unique_ptr<A> a;
};

// main.cpp
// #include "Afile.hpp"
// #include "BFile.hpp"

int A::getHoge(B::Hoge hoge)
{
    return hoge.huga;
}

int main()
{
    B b;
}
