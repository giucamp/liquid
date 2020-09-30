
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

namespace liquid
{
    using Real = double;
    using Integer = int64_t;
    using Bool = bool;

    enum class ScalarType { Any, Real, Integer, Bool };

    template <typename TYPE>
        constexpr ScalarType GetScalarType()
    {
        if constexpr (std::is_same_v<TYPE, Real>)
            return ScalarType::Real;
        else if constexpr (std::is_same_v<TYPE, Integer>)
            return ScalarType::Integer;
        else if constexpr (std::is_same_v<TYPE, Bool>)
            return ScalarType::Bool;
    }

    namespace detail
    {
        template <ScalarType> struct FromScalarTypeImpl;
        template <> struct FromScalarTypeImpl<ScalarType::Real> { using type = Real; };
        template <> struct FromScalarTypeImpl<ScalarType::Integer> { using type = Integer; };
        template <> struct FromScalarTypeImpl<ScalarType::Bool> { using type = Bool; }; 
    }
    template <ScalarType SCALAR_TYPE>
        using FromScalarType = typename detail::FromScalarTypeImpl<SCALAR_TYPE>::type;

    std::ostream & operator << (std::ostream & i_ostream, ScalarType i_scalar_type);

    template <typename... TYPE>
        std::string ToString(const TYPE & ... i_object)
    {
        std::ostringstream stream;
        (stream << ... << i_object);
        return stream.str();
    }

    void DbgBreak();

    template <typename... TYPE>
        [[noreturn]] void Panic(const TYPE & ... i_object)
    {
        std::string message = ToString(i_object...);
        std::cerr << message << std::endl;
        DbgBreak();
        throw std::exception(message.c_str());
    }

    inline bool AssertCheck(bool i_value) { return i_value; }
    #define LIQUID_ASSERT(expr) if(!liquid::AssertCheck(expr)) Panic("Assert failure: " #expr);

    template <typename DEST_TYPE, typename SOURCE_TYPE>
        DEST_TYPE NumericCast(SOURCE_TYPE i_source)
    {
        auto const result = static_cast<DEST_TYPE>(i_source);
        auto const source_back = static_cast<SOURCE_TYPE>(result);
        auto const source_negative = i_source < 0;
        auto const result_negative = result < 0;
        if(i_source != source_back || source_negative != result_negative)
            Panic("NumericCast - Bad cast of ", i_source);
        return result;
    }

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
}
