
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
    void TestPow()
    {
        std::cout << "Test Pow...";
        const char topic[] = "Pow";

        Expects(topic, "3^2 == 9");

        // ^ is right-associative
        //Expects(topic, "real x ^ real y ^ real z == x^(y^x)");
        
        Expects(topic, "2 + 3 + 2 == 7.");
        Expects(topic, "add() == 0");
        Expects(topic, "add(5 6 5) == 16");
        Expects(topic, "add([5 6 5]) == [5 6 5]");
        Expects(topic, "add([5 6 5], 1) == [6 7 6]");

        std::cout << "done" << std::endl;
    }
}
