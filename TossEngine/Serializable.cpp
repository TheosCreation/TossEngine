#include "Serializable.h"

void Serializable::FloatSlider(const std::string& name, float& value, float speed, float min, float max)
{
    ImGui::DragFloat(name.c_str(), &value, speed, min, max);
}
