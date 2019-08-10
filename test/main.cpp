#include <iostream>
#include "B.hpp"
#include "A.hpp"

int main()
{
    B b;
    B::Hoge hoge;
    hoge.huga = 1;
    std::cout << b.a->getHoge(hoge) << std::endl;
}
