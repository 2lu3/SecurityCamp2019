#ifndef A_HPP
#define A_HPP
#include <memory>

#include "B.hpp"
class B;

class A
{
public:
    int getHoge(B::Hoge &hoge);
};

#endif
