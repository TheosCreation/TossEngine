#include "ComponentRegistry.h"

#ifdef _WIN32
#include <windows.h>
#endif

void ComponentRegistry::addComponentModule(const std::type_info& typeInfo)
{
    std::string typeName = getClassName(typeInfo);  // Always get the type name

    void* moduleHandle = nullptr;

#ifdef _WIN32
    // Get module (DLL) handle from the RTTI object address
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)&typeInfo, &mbi, sizeof(mbi))) {
        moduleHandle = mbi.AllocationBase;
    }
#endif

    m_componentModules[typeName] = moduleHandle;
}

void ComponentRegistry::CleanUpModule(void* moduleHandle)
{
    for (auto it = m_componentModules.begin(); it != m_componentModules.end(); )
    {
        if (it->second == moduleHandle)
        {
            std::string typeName = it->first;
            Debug::Log("Unregistering component: " + typeName);

            m_componentCreators.erase(typeName);
            m_componentTypes.erase(typeName);
            it = m_componentModules.erase(it); // remove from modules and move iterator forward
        }
        else
        {
            ++it;
        }
    }
}

std::vector<std::string> ComponentRegistry::getRegisteredComponentNames() const
{
    std::vector<std::string> names;
    for (const auto& pair : m_componentCreators)
    {
        names.push_back(pair.first);
    }
    return names;
}

void ComponentRegistry::CleanUp()
{
    m_componentCreators.clear();
}
