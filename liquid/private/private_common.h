
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include <functional>
#include "liquid/liquid_common.h"
#include "liquid/tensor.h"

namespace liquid
{
    template <typename FLAGS, typename = std::enable_if_t<std::is_enum_v<FLAGS>>>
        constexpr FLAGS CombineFlags(FLAGS i_first, FLAGS i_second)
    {
        return static_cast<FLAGS>(static_cast<int>(i_first) | static_cast<int>(i_second));
    }

    template <typename FLAGS, typename = std::enable_if_t<std::is_enum_v<FLAGS>>>
        constexpr bool HasFlags(FLAGS i_all, FLAGS i_some)
    {
        return (static_cast<int>(i_all) & static_cast<int>(i_some)) == static_cast<int>(i_some);
    }

    void Expects(const char * i_topic, const Tensor & i_bool_tensor,
        const char * i_cpp_source_code);

    void Expects(const char * i_topic, const char * i_miu6_source_code);

    void Expects(const char * i_miu6_source_code);

    void ExpectsPanic(const char * i_topic, 
        const std::function<void()> & i_function,
        const char * i_cpp_source_code, const char * i_expected_message);

    void ExpectsPanic(const char * i_topic, const char * i_miu6_source_code, 
        const char * i_expected_message);

    void ExpectsPanic(const char * i_miu6_source_code, 
        const char * i_expected_message);

    #define LIQUID_ASSERT(expr) if(!(expr)) Panic("Assert failure: " #expr);

    #define LIQUID_EXPECTS(expr) ::liquid::Expects(nullptr, expr, #expr)

    #define LIQUID_EXPECTS_DOC(topic, expr) ::liquid::Expects(topic, expr, #expr)

    #define LIQUID_EXPECTS_PANIC(expr, expected_error) \
        ::liquid::ExpectsPanic(nullptr, [&]{ (void)(expr); }, #expr, expected_error);

    #define LIQUID_EXPECTS_PANIC_DOC(topic, expr, expected_error) \
        ::liquid::ExpectsPanic(topic, [&]{ (void)(expr); }, #expr, expected_error);
}
