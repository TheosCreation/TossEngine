#include "Vector4.h"
#include "Vector3.h"

// Constructor that promotes a Vector3 and a W value
constexpr Vector4::Vector4(const Vector3& v, float w)
    : x(v.x), y(v.y), z(v.z), w(w) {
}

// Computes the magnitude (length) of the vector
float Vector4::Length() const {
    return glm::length(glm::vec4(x, y, z, w));
}

// Returns a normalized copy (unit length vector)
Vector4 Vector4::Normalized() const {
    glm::vec4 normalized = glm::normalize(glm::vec4(x, y, z, w));
    return Vector4(normalized);
}

// Normalizes the current vector in-place
void Vector4::Normalize() {
    *this = this->Normalized();
}

// Converts the vector to a human-readable string
std::string Vector4::ToString() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ", " << w << ")";
    return oss.str();
}

// Unary minus operator (negate all components)
Vector4 Vector4::operator-() const {
    return Vector4(-x, -y, -z, -w);
}

// Subtract another vector from this vector
Vector4& Vector4::operator-=(const Vector4& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

// Add another vector to this vector
Vector4 Vector4::operator+(const Vector4& other) const {
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

// Add another vector to this vector (in-place)
Vector4& Vector4::operator+=(const Vector4& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

// Subtract another vector (non-in-place)
Vector4 Vector4::operator-(const Vector4& other) const {
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

// Multiply vector by scalar
Vector4 Vector4::operator*(float scalar) const {
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

// Multiply vector by scalar (in-place)
Vector4& Vector4::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

// Divide vector by scalar
Vector4 Vector4::operator/(float scalar) const {
    return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

// Divide vector by scalar (in-place)
Vector4& Vector4::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

// Equality comparison
bool Vector4::operator==(const Vector4& other) const {
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

// Inequality comparison
bool Vector4::operator!=(const Vector4& other) const {
    return !(*this == other);
}
