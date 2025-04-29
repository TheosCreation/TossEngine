#pragma once
#include <nlohmann/json.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include "Utils.h"

// --- Utility Macros for Serializable Classes ---

// Helper: Expand and stringify an identifier
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

// --- Internal macros used by the public SERIALIZABLE_MEMBERS ---

// Serialize one member into JSON
#define _SER_MEMBER(r, j, member) \
    j[STRINGIZE(member)] = this->member;

// Deserialize one member from JSON
#define _DESER_MEMBER(r, j, member) \
    if ((j).contains(STRINGIZE(member)) && !(j)[STRINGIZE(member)].is_null()) \
        (j).at(STRINGIZE(member)).get_to(this->member);

// --- Public Macro: Declares automatic serialize()/deserialize() methods for the given fields ---

// Usage: SERIALIZABLE_MEMBERS(field1, field2, field3)
#define SERIALIZABLE_MEMBERS(...) \
    SERIALIZABLE_MEMBERS_IMPL(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

// Implementation details
#define SERIALIZABLE_MEMBERS_IMPL(seq)                                \
public:                                                               \
    inline nlohmann::json serialize() const override {                \
        nlohmann::json j = Serializable::serialize();                 \
        BOOST_PP_SEQ_FOR_EACH(_SER_MEMBER, j, seq)                    \
        return j;                                                     \
    }                                                                 \
    inline void deserialize(const nlohmann::json& j) override {       \
        Serializable::deserialize(j);                                 \
        BOOST_PP_SEQ_FOR_EACH(_DESER_MEMBER, j, seq)                  \
    }