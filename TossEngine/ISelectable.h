#pragma once
#include "Utils.h"

class TOSSENGINE_API ISelectable
{
public:
    virtual void OnInspectorGUI() {}
    virtual void OnSelect() {}
    virtual void OnDeSelect() {}
    virtual bool Delete(bool deleteSelf = true) { return false;  }


};

