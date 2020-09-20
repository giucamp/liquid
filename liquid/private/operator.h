
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "liquid.h"
#include <functional>
#include <string_view>

namespace liquid
{
    class Operator
    {
    public:
 
        enum class Flags
        {
            None = 0,
            Commutative = 1 << 0,
            Associative = 1 << 1
        };

        friend Flags operator | (Flags i_first, Flags i_second)
        {
            return static_cast<Flags>(static_cast<int>(i_first) | static_cast<int>(i_second));
        }

        using SimplifyFunction = std::function<Tensor (const Tensor)>;

        Operator(std::string_view i_name);

        Operator & AddFlags(Flags i_flags);

        Operator & SetSimplifyFunction(const SimplifyFunction & i_func);

    private:
        std::string const m_name;
        Flags m_flags{};
        SimplifyFunction m_simpily_func;
    };
}
