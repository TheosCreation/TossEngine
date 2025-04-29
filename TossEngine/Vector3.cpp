#include "Vector3.h"
#include "Vector2.h"
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

// Vector2 -> Vector3 conversion (z = 0)
Vector3::Vector3(const Vector2& vec) : x(vec.x), y(vec.y), z(0.0f) {}

float Vector3::Length() const {
    return glm::length(glm::vec3(x, y, z));
}

Vector3 Vector3::Normalized() const {
    return Vector3(glm::normalize(glm::vec3(x, y, z)));
}

void Vector3::Normalize() {
    *this = this->Normalized();
}

Vector3 Vector3::Cross(const Vector3& a, const Vector3& b) {
    return Vector3(glm::cross(glm::vec3(a), glm::vec3(b)));
}

float Vector3::Distance(const Vector3& a, const Vector3& b) {
    return glm::distance(glm::vec3(a), glm::vec3(b));
}

Vector3 Vector3::ExtractTranslation(const Mat4& m) {
    return Vector3(m.value[3][0], m.value[3][1], m.value[3][2]);
}

Vector3 Vector3::ExtractScale(const Mat4& m) {
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(m.value[0][0], m.value[0][1], m.value[0][2]));
    scale.y = glm::length(glm::vec3(m.value[1][0], m.value[1][1], m.value[1][2]));
    scale.z = glm::length(glm::vec3(m.value[2][0], m.value[2][1], m.value[2][2]));
    return Vector3(scale);
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

Vector3 Vector3::ToDegrees() const {
    return Vector3(glm::degrees(glm::vec3(x, y, z)));
}

Vector3 Vector3::ToRadians() const {
    return Vector3(glm::radians(glm::vec3(x, y, z)));
}

std::string Vector3::ToString() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
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
Vector3 Vector3::operator*(const Quaternion& q) const {
    glm::vec3 rotated = glm::rotate(glm::quat(q), glm::vec3(x, y, z));
    return Vector3(rotated);
}