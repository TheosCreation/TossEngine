#pragma once
#include "Generated/TestClass2.rfkh.h"

#include "TestClass.h"

CLASS()
class TOSSENGINE_API TestClass2 : public TestClass
{
public:
    TestClass2() = default;
    ~TestClass2() = default;

    FIELD() int _intField5 = 30;

    TestClass2_GENERATED
};


File_TestClass2_GENERATED