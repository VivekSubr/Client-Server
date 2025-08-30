#pragma once

class TestInterface 
{
public:
    virtual ~TestInterface() {};

    virtual void Func1() = 0;
    virtual bool Func2() = 0;
};