#pragma once
#include <nlohmann/json.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include "Utils.h"

//—— per-member serializers ——
#define _SER_MEMBER(r, j, member) \
    j[#member] = this->member;

#define _DESER_MEMBER(r, j, member)                      \
    if ((j).contains(#member) && ! (j)[#member].is_null()) \
        (j).at(#member).get_to(this->member);

//—— step 1: user-facing macro ——
#define SERIALIZABLE_MEMBERS(...) \
    _SERIALIZABLE_MEMBERS_IMPL( BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) )

//—— step 2: implementation macro ——
#define _SERIALIZABLE_MEMBERS_IMPL(seq)                                             \
    inline nlohmann::json serialize() const override {                              \
        nlohmann::json j = Serializable::serialize();                                \
        BOOST_PP_SEQ_FOR_EACH(_SER_MEMBER, j, seq)                                   \
        return j;                                                                    \
    }                                                                                \
    inline void deserialize(const nlohmann::json& j) override {                     \
        Serializable::deserialize(j);                                                \
        BOOST_PP_SEQ_FOR_EACH(_DESER_MEMBER, j, seq)                                 \
    }
