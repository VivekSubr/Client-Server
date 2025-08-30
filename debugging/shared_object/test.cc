#include "test.h"
#include <iostream>

void TestImpl::Func1() 
{
    std::cout<<"func1\n";
}

bool TestImpl::Func2()
{
    std::cout<<"func2\n";
    return false;
}