#pragma once
#include "Generated/TestClass.rfkh.h"

#include "Utils.h"


CLASS()
class TOSSENGINE_API TestClass
{
public:
    TestClass() = default;
    ~TestClass() = default;

    FIELD() int _intField = 10;
    FIELD() int _intField2 = 10;

    TestClass_GENERATED
}; 

File_TestClass_GENERATED