
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "indices.h"
#include <iostream>

namespace liquid
{
    void TestMul()
    {
        std::cout << "Test Mul...";
        const char topic[] = "Mul";

        Expects(topic, "mul() == 1");
        Expects(topic, "mul(real a, real b, real c) == a * b * c");
        Expects(topic, "mul(mul(real a, real b), real c) == a * b * c");
        Expects(topic, "real a * real b * real c == c * b * a");
        Expects(topic, "real a * 0 == 0");
        Expects(topic, "real a * 1 == a");
        Expects(topic, "real a / real b == a * b^-1");
        Expects(topic, "(real a ^ real b) * (a ^ real c) == a^(b + c)");
        Expects(topic, "(real a ^ real b) ^ real c == a^(b * c)");
        // Expects(topic, "real a / a == 1");
        // Expects(topic, "real a + a == 2 * a");

        Expects(topic, "2 * 3 == 6");
        Expects(topic, "2 * 3 * 2 == 12");
        Expects(topic, "mul(5 6 5) == 150");
        Expects(topic, "mul([5 6 5]) == [5 6 5]");
        Expects(topic, "add([5 6 5], 1) == [6 7 6]");

        std::cout << "done" << std::endl;
    }
}
