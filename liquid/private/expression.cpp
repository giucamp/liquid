
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"

namespace liquid
{
    bool AlwaysEqual(const Tensor& i_tensor, const TensorValue& i_value)
    {
        return IsConstant(i_tensor) && GetConstantValue(i_tensor) == i_value;
    }
}
