
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include <iostream>

namespace liquid
{
    void TestScalarType()
    {
        std::cout << "Test ScalarType...";

        const char topic[] = "scalar types";

        LIQUID_EXPECTS_DOC(topic, GetScalarType<Real>() == ScalarType::Real);
        LIQUID_EXPECTS_DOC(topic, GetScalarType<Integer>() == ScalarType::Integer);
        LIQUID_EXPECTS_DOC(topic, GetScalarType<Bool>() == ScalarType::Bool);

        static_assert(std::is_same_v<FromScalarType<ScalarType::Real>, Real>);
        static_assert(std::is_same_v<FromScalarType<ScalarType::Integer>, Integer>);
        static_assert(std::is_same_v<FromScalarType<ScalarType::Bool>, Bool>);

        std::cout << "done" << std::endl;
    }
}
