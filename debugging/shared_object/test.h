#pragma once
#include "test_inf.h"

class TestImpl : public TestInterface
{
public:
    void Func1() override;
    bool Func2() override;
};