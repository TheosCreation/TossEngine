#pragma once
#include <nlohmann/json.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include "Utils.h"

// two‑step stringizer so identifiers get expanded before the “#”
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x)        STRINGIZE_DETAIL(x)

// per‑member macros
#define _SER_MEMBER(r, j, member) \
    j[STRINGIZE(member)] = this->member;

#define _DESER_MEMBER(r, j, member)                                \
    if ((j).contains(STRINGIZE(member)) && !(j)[STRINGIZE(member)].is_null()) \
        (j).at(STRINGIZE(member)).get_to(this->member);

// unwrap the variadic into a true Boost‑PP seq before iterating
#define SERIALIZABLE_MEMBERS(...) \
    SERIALIZABLE_MEMBERS_IMPL( BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) )

#define SERIALIZABLE_MEMBERS_IMPL(seq)                                \
  inline nlohmann::json serialize() const override {                  \
    nlohmann::json j = Serializable::serialize();                     \
    BOOST_PP_SEQ_FOR_EACH(_SER_MEMBER, j, seq)                        \
    return j;                                                         \
  }                                                                   \
  inline void deserialize(const nlohmann::json& j) override {        \
    Serializable::deserialize(j);                                     \
    BOOST_PP_SEQ_FOR_EACH(_DESER_MEMBER, j, seq)                      \
  }
