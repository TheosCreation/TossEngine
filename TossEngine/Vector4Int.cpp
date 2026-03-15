#include "Vector4Int.h"

std::string Vector4Int::ToString() const
{
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ", " << w << ")";
    return oss.str();
}
