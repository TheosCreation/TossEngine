#pragma once
#include "Utils.h"
#include <boost/preprocessor.hpp>

class TOSSENGINE_API Serializable
{
public:
    Serializable() = default;

    // Serialize to JSON
    virtual json serialize() const
    {
        return {
            {"type", getClassName(typeid(*this))} // used to identify class type
        };
    }

    // Deserialize from JSON
    virtual void deserialize(const json& data) { }
};

// Helper to emit:   j["field"] = field;
#define _SERIALIZE_FIELD(r, _unused, field)    j[#field] = field;

// Helper to emit:   j.at("field").get_to(field);
#define _DESERIALIZE_FIELD(r, _unused, field)  j.at(#field).get_to(field);

// The user‐facing macro. List your member names in the parentheses.
#define SERIALIZE_COMPONENT_FIELDS(...)                              \
  nlohmann::json serialize() const override {                        \
    auto j = Component::serialize();                                 \
    BOOST_PP_SEQ_FOR_EACH(_SERIALIZE_FIELD, _,                         \
                         BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))      \
    return j;                                                        \
  }                                                                  \
  void deserialize(const nlohmann::json& j) override {               \
    Component::deserialize(j);                                       \
    BOOST_PP_SEQ_FOR_EACH(_DESERIALIZE_FIELD, _,                       \
                         BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))      \
  }
