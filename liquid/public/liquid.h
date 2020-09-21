
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
        DbgBreak();
        throw std::exception(message.c_str());
    }

    #define LIQUID_ASSERT(expr) if(!(expr)) Panic("Assert failure: " #expr);

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
}

#include "tensor.h"
