
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstdint>
#include <string>
#include <sstream>
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

    [[noreturn]] void Panic(const std::string & i_error);

    template <typename... TYPE>
        [[noreturn]] void Panic(const TYPE & ... i_object)
    {
        Panic(ToString(i_object...));
    }

    template <typename DEST_TYPE, typename SOURCE_TYPE>
        DEST_TYPE NumericCast(SOURCE_TYPE i_source)
    {
        if constexpr(std::is_same_v<DEST_TYPE, SOURCE_TYPE>)
        {
            // source type and dest type are the same, no loss can occur
            return i_source;
        }
        else if constexpr(std::is_signed_v<DEST_TYPE> == std::is_signed_v<SOURCE_TYPE>)
        {
            // source type and dest type are both signed or unsigned, don't check the sign
            auto const result = static_cast<DEST_TYPE>(i_source);
            auto const source_back = static_cast<SOURCE_TYPE>(result);
            if(i_source != source_back)
                Panic("NumericCast - Bad cast of ", i_source);
            return result;
        }
        else
        {
            // general case
            auto const result = static_cast<DEST_TYPE>(i_source);
            auto const source_back = static_cast<SOURCE_TYPE>(result);
            auto const source_negative = i_source < 0;
            auto const result_negative = result < 0;
            if(i_source != source_back || source_negative != result_negative)
                Panic("NumericCast - Bad cast of ", i_source);
            return result;
        }
    }

    template <typename FIRST, typename...>
        using FirstOf = FIRST;
}
