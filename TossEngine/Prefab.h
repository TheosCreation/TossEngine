#pragma once
#include "GameObject.h"
#include "Resource.h"

class TOSSENGINE_API Prefab : public GameObject, public Resource
{
public:
    Prefab(const std::string& uniqueID, ResourceManager* manager);
    void OnInspectorGUI() override;
};

