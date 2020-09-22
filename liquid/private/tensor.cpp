
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid.h"
#include "expression.h"

namespace liquid
{
    bool Always(const Tensor& i_bool_tensor)
    {
        return AlwaysEqual(i_bool_tensor, constant_values::True);
    }

    bool Never(const Tensor& i_bool_tensor)
    {
        return AlwaysEqual(i_bool_tensor, constant_values::False);
    }
}
