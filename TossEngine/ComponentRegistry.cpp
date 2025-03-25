#include "ComponentRegistry.h"

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
