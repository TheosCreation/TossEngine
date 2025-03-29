#pragma once
#include "Utils.h"

class TOSSENGINE_API ISelectable
{
public:
    virtual void OnInspectorGUI() {}
    virtual bool Delete(bool deleteSelf = true) { return false;  }


};

