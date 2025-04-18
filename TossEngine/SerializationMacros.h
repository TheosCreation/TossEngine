#pragma once
#include <nlohmann/json.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include "Utils.h"

#define _SER_MEMBER(r, j, member)     j[#member] = this->member;
#define _DESER_MEMBER(r, j, member)                         \
    if (j.contains(#member) && !j[#member].is_null())       \
        j.at(#member).get_to(this->member);

#define SERIALIZABLE_MEMBERS(...)                                \
    json serialize() const override {                            \
        json j = Serializable::serialize();                      \
        BOOST_PP_SEQ_FOR_EACH(                                   \
          _SER_MEMBER,                                           \
          j,                                                     \
          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                 \
        );                                                       \
        return j;                                                \
    }                                                            \
    void deserialize(const json& j) override {                   \
        Serializable::deserialize(j);                            \
        BOOST_PP_SEQ_FOR_EACH(                                   \
          _DESER_MEMBER,                                         \
          j,                                                     \
          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                 \
        );                                                       \
    }