
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/liquid_common.h"
#include "indices.h"
#include <numeric>
#include <iostream>

namespace liquid
{
    void TestIf()
    {
        std::cout << "Test If...";

        {
            Tensor t2 = 2.0;
            Tensor t3 = 3.0;
            Tensor u3 = 3.0;

            LIQUID_ASSERT(t3 > t2);
            LIQUID_ASSERT(t2 < t3);
            LIQUID_ASSERT(t3 >= t2);
            LIQUID_ASSERT(t2 <= t3);
            LIQUID_ASSERT(t2 != t3);

            LIQUID_ASSERT(t3 == u3);
            LIQUID_ASSERT(t3 <= u3);
            LIQUID_ASSERT(t3 >= u3);
        }

        std::cout << "done" << std::endl;
    }
}
