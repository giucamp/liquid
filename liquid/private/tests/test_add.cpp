
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "indices.h"
#include <iostream>

namespace liquid
{
    void TestAdd()
    {
        std::cout << "Test Add...";
        const char topic[] = "Add";

        Expects(topic, "add() == 0");
        Expects(topic, "add(real a, real b, real c) == a + b + c");
        Expects(topic, "add(add(real a, real b), real c) == a + b + c");
        Expects(topic, "real a + real b + real c == c + b + a");
        Expects(topic, "real a + 0 == a");

        Expects(topic, "real a + a == 2*a");

        Expects(topic, "2 + 3 == 5");
        Expects(topic, "2 + 3 + 2 == 7");
        Expects(topic, "add(5 6 5) == 16");
        Expects(topic, "add([5 6 5]) == [5 6 5]");
        Expects(topic, "add([5 6 5], 1) == [6 7 6]");

        std::cout << "done" << std::endl;
    }
}
