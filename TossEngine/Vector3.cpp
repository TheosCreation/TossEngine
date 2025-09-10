#include "Vector3.h"

#include "reactphysics3d/mathematics/Vector3.h"
#include "Vector2.h"
#include "Vector4.h"
#include "Mat4.h"
#include "Quaternion.h"

// Static constant definitions
const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Back = Vector3(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);

Vector3::Vector3(const reactphysics3d::Vector3& vec) : x(vec.x), y(vec.y), z(vec.z) {}

// Vector2 -> Vector3 conversion (z = 0)
Vector3::Vector3(const Vector2& vec) : x(vec.x), y(vec.y), z(0.0f) {}

Vector3::Vector3(const Vector4& vec) : x(vec.x), y(vec.y), z(vec.z) {}

float Vector3::Length() const
{
#if defined(_WIN32)
    return glm::length(glm::vec3(x, y, z));
#elif defined(__PROSPERO__)
    return 0.0f; //TODO: Implement
#endif
    return 0.0f;
}

Vector3 Vector3::Normalized() const
{
#if defined(_WIN32)
    return Vector3(glm::normalize(glm::vec3(x, y, z)));
#elif defined(__PROSPERO__)
    return Vector3(); //TODO: Implement
#endif
    return Vector3();
}

void Vector3::Normalize() {
    *this = this->Normalized();
}

Vector3 Vector3::Cross(const Vector3& a, const Vector3& b)
{
#if defined(_WIN32)
    return Vector3(glm::cross(glm::vec3(a), glm::vec3(b)));
#elif defined(__PROSPERO__)
    return Vector3(); //TODO: Implement
#endif
    return Vector3();
}

float Vector3::Distance(const Vector3& a, const Vector3& b)
{
#if defined(_WIN32)
    return glm::distance(glm::vec3(a), glm::vec3(b));
#elif defined(__PROSPERO__)
    return 0.0f; //TODO: Implement
#endif
    return 0.0f;
}

Vector3 Vector3::ExtractTranslation(const Mat4& m) {
    return Vector3(m[3]);
}

Vector3 Vector3::ExtractScale(const Mat4& m) {
#if defined(_WIN32)
    Vector3 scale;
    scale.x = Vector3(m[0].x, m[0].y, m[0].z).Length();
    scale.y = Vector3(m[1].x, m[1].y, m[1].z).Length();
    scale.z = Vector3(m[2].x, m[2].y, m[2].z).Length();
    return scale;
#elif defined(__PROSPERO__)
    return Vector3(); //TODO: Implement
#endif
    return Vector3();
}

Vector3 Vector3::Lerp(Vector3 start, Vector3 end, float t) {
    t = std::clamp(t, 0.0f, 1.0f); // Clamp between 0 and 1
    return start + (end - start) * t;
}

Mat4 Vector3::ToTranslation() const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    return Mat4(translation);
}

Mat4 Vector3::ToScale() const {
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z));
    return Mat4(scale);
}

bool Vector3::Equals(const Vector3& other, float epsilon) const {
    return glm::all(glm::epsilonEqual(glm::vec3(x, y, z), glm::vec3(other.x, other.y, other.z), epsilon));
}

Vector3 Vector3::ToDegrees() const
{
#if defined(_WIN32)
    return Vector3(glm::degrees(glm::vec3(x, y, z)));
#elif defined(__PROSPERO__)
    return Vector3(); //TODO: Implement
#endif
    return Vector3();
}

Vector3 Vector3::ToRadians() const
{
#if defined(_WIN32)
    return Vector3(glm::radians(glm::vec3(x, y, z)));
#elif defined(__PROSPERO__)
    return Vector3(); //TODO: Implement
#endif
    return Vector3();
}

std::string Vector3::ToString() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
}

Vector3::operator reactphysics3d::Vector3() const
{
    return reactphysics3d::Vector3(x, y, z);
}

Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}

Vector3& Vector3::operator-=(const Vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3& Vector3::operator+=(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3 Vector3::operator*(float scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3& Vector3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3 Vector3::operator*(const Vector3& other) const {
    return Vector3(x * other.x, y * other.y, z * other.z);
}

Vector3& Vector3::operator*=(const Vector3& other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

Vector3 Vector3::operator/(const Vector3& other) const {
    return Vector3(x / other.x, y / other.y, z / other.z);
}

Vector3& Vector3::operator/=(const Vector3& other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

bool Vector3::operator==(const Vector3& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool Vector3::operator!=(const Vector3& other) const {
    return !(*this == other);
}

// Multiply Vector3 by a Quaternion (rotation)
Vector3 Vector3::operator*(const Quaternion& q) const
{
#if defined(_WIN32)
    glm::vec3 rotated = glm::rotate(glm::quat(q), glm::vec3(x, y, z));
    return Vector3(rotated);
#elif defined(__PROSPERO__)
    return Vector3(); //TODO: Implement
#endif
    return Vector3();
}