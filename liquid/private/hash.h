
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "liquid/span.h"
#include <type_traits>
#include <string_view>
#include <string>

namespace liquid
{
    // machine dependent hash
    class Hash
    {
    public:

        // Hash(values...)
        template <typename... TYPE>
            explicit Hash(const TYPE & ... i_object)
                { ((*this) << ... << i_object); }

        Hash & operator << (Span<const unsigned char> i_data)
        {
            // http://www.cse.yorku.ca/~oz/hash.html
            for(auto c : i_data)
                m_value = m_value * 33 + static_cast<uint64_t>(c);
            return *this;
        }

        uint64_t GetValue() const    { return m_value; }

        bool operator == (const Hash & i_right) const
            { return m_value == i_right.m_value; }

        bool operator != (const Hash & i_right) const
            { return m_value != i_right.m_value; }

        bool operator < (const Hash & i_right) const
            { return m_value < i_right.m_value; }

        bool operator > (const Hash & i_right) const
            { return m_value > i_right.m_value; }

        bool operator <= (const Hash & i_right) const
            { return m_value <= i_right.m_value; }

        bool operator >= (const Hash & i_right) const
            { return m_value >= i_right.m_value; }

    private:
        uint64_t m_value = 5381;
    };

    template <typename TYPE, typename = std::enable_if_t<
            std::is_arithmetic_v<TYPE> || std::is_enum_v<TYPE> >>
        Hash & operator << (Hash & i_dest, const TYPE & i_object)
    {
        // type-aliasing rule is not violated becuse we inspect the object with unsigned chars
        // https://en.cppreference.com/w/cpp/language/reinterpret_cast#Type_aliasing
        auto const address = reinterpret_cast<const unsigned char*>(&i_object);
        return i_dest << Span(address, sizeof(TYPE));
    }

    inline Hash & operator << (Hash & i_dest, const Hash & i_source)
    {
        i_dest << i_dest.GetValue();
        return i_dest;
    }

    inline Hash & operator << (Hash & i_dest, std::string_view i_src)
    {
        auto const address = reinterpret_cast<const unsigned char*>(i_src.data());
        return i_dest << Span(address, i_src.length());
    }

    template <typename CONTAINER>
        FirstOf<Hash, ContainerElementTypeT<CONTAINER>> & operator << 
            (Hash & i_dest, const CONTAINER & i_container)
    {
        for(const auto & element : i_container)
            i_dest << element;
        return i_dest;
    }
}
