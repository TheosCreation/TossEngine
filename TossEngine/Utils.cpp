#include "Utils.h"



template <>
std::string ToString<RenderingPath>(const RenderingPath& value) {
    switch (value)
    {
    case RenderingPath::Deferred: return "Deferred Rendering";
    case RenderingPath::Forward: return "Forward Rendering";
    default: return "Unknown";
    }
}

template <>
RenderingPath FromString<RenderingPath>(const std::string& input) {
    static const std::unordered_map<std::string, RenderingPath> enumMap = {
        {"Forward Rendering", RenderingPath::Forward},
        {"Deferred Rendering", RenderingPath::Deferred}
    };

    auto it = enumMap.find(input);
    return (it != enumMap.end()) ? it->second : RenderingPath::Unknown;
}

template<typename T>
T FromString(const std::string& input)
{
    std::stringstream ss(input);
    T value;

    // Handle boolean values
    if constexpr (std::is_same_v<T, bool>) {
        if (input == "true") return true;
        if (input == "false") return false;
        return static_cast<T>(std::stoi(input) != 0); // Handle "1"/"0"
    }
    // Handle basic types like int, float, double
    else {
        ss >> value;
    }

    return value;
}