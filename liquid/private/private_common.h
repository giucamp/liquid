
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

    template <typename TYPE>
        std::vector<TYPE> & Append(std::vector<TYPE> & i_dest, Span<const TYPE> i_source)
    {
        i_dest.insert(i_dest.end(), i_source.begin(), i_source.end());
        return i_dest;
    }

    // Contains - less verbose than using std::find, and does not require <algorithm>
    template <typename CONTAINER>
        bool Contains(const CONTAINER & i_container, const ContainerElementTypeT<CONTAINER> & i_value)
    {
        for(const auto & element : i_container)
            if(element == i_value)
                return true;
        return false;
    }

    /** Concatenate many containers into a vector. This function does not partecipate in
        overload resolution unless all argument are containers with the same element type. */
    template <
        typename FIRST_PIECE, typename... OTHER_PIECES,    
        typename ELEMENT_TYPE = ContainerElementTypeT<FIRST_PIECE>,
        typename = std::enable_if_t< IsContainerOfV<FIRST_PIECE, ELEMENT_TYPE>
                && (IsContainerOfV<OTHER_PIECES, ELEMENT_TYPE> && ...) > 
            > auto Concatenate(const FIRST_PIECE & i_first_piece, const OTHER_PIECES & ... i_pieces)
    {
        auto const size = (
            std::size(i_first_piece) + ... + std::size(i_pieces) );

        std::vector<ELEMENT_TYPE> result;
        result.reserve(size);

        result.insert(result.end(), i_first_piece.begin(), i_first_piece.end());
        (result.insert(result.end(), i_pieces.begin(), i_pieces.end()), ...);
        return result;
    }

    template <typename SOURCE_CONTAINER, typename PREDICATE>
        auto Transform(const SOURCE_CONTAINER & i_source_elements, const PREDICATE & i_predicate)
    {
        using DestType = decltype(i_predicate(*std::declval<SOURCE_CONTAINER>().begin()));
        std::vector<DestType> result;
        auto const size = std::size(i_source_elements);
        result.reserve(size);
        for(const auto & source : i_source_elements)
            result.push_back(i_predicate(source));
        return result;
    }

    inline bool StartsWith(std::string_view i_source, std::string_view i_prefix)
    {
        return i_source.length() >= i_prefix.length() &&
            i_source.substr(0, i_prefix.length()) == i_prefix;
    }

    class SilentPanicContext
    {
    public:

        SilentPanicContext();
        ~SilentPanicContext();

        SilentPanicContext(const SilentPanicContext &) = delete;
        SilentPanicContext & operator = (const SilentPanicContext &) = delete;
    };

    #define LIQUID_ASSERT(expr) if(!(expr)) Panic("Assert failure: " #expr);

    #define LIQUID_EXPECTS(expr) ::liquid::Expects(nullptr, expr, #expr)

    #define LIQUID_EXPECTS_DOC(topic, expr) ::liquid::Expects(topic, expr, #expr)

    #define LIQUID_EXPECTS_PANIC(expr, expected_error) \
        ::liquid::ExpectsPanic(nullptr, [&]{ (void)(expr); }, #expr, expected_error);

    #define LIQUID_EXPECTS_PANIC_DOC(topic, expr, expected_error) \
        ::liquid::ExpectsPanic(topic, [&]{ (void)(expr); }, #expr, expected_error);
}
