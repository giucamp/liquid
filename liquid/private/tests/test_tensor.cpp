
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
    void TestTensor()
    {
        std::cout << "TestTensor...";

        {
            Tensor t(2.0);
            LIQUID_ASSERT(t + 3.0 == 5.0); 
        }

        {
            Tensor t({1.0, 2.0});
        }

        std::cout << "done" << std::endl;
    }
}
