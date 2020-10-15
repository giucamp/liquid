
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
    void TestIs()
    {
        std::cout << "Test Is...";
        const char topic[] = "Is";

        {
            Expects(topic, "any is any");
            Expects(topic, "int is any");
            Expects(topic, "real is real");
            Expects(topic, "Not real is int");
        }

        std::cout << "done" << std::endl;
    }
}
