#include "Utils.h"



template <>
std::string ToString<RenderingPath>(const RenderingPath& value) {
    switch (value)
    {
    case RenderingPath::Deferred: return "Deferred Rendering";
    case RenderingPath::Forward: return "Forward Rendering";
    default: return "Unknown Rendering Path";
    }
}