
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "private_common.h"
#include "liquid/tensor.h"

namespace liquid
{
    class Scope
    {
    public:

    private:
        std::vector<Rule> m_rules;
        std::vector<Tensor> m_values;
    };
}
