
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
            Expects(topic, "4 is int");
            Expects(topic, "5.2 is real");
            Expects(topic, "1/2 is real");
            Expects(topic, "int is any");
            Expects(topic, "any is any");
            Expects(topic, "real is real");
            Expects(topic, "not real is bool");

            Expects(topic, "(real == real) is bool");
            Expects(topic, "(rank of any) is int[]");
            Expects(topic, "(any is real) is bool");

            Expects(topic, "real a * int b is real");
            Expects(topic, "int / int is real");
            Expects(topic, "any is any is bool[]");

            // Expects(topic, "real is real or not any is real");
            
            Expects(topic, "real is any and any is any");
        }

        std::cout << "done" << std::endl;
    }
}
