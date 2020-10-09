
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "indices.h"
#include <numeric>
#include <iostream>

namespace liquid
{
    void TestAdd()
    {
        std::cout << "Test Add...";
        const char topic[] = "Add";

        Expects(topic, "2 + 3 == 5");

        std::cout << "done" << std::endl;
    }
}
