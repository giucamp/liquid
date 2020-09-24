
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/liquid_common.h"
#include <iostream>

namespace liquid
{
    void TestScalarType()
    {
        std::cout << "TestScalarTypefd...";

        LIQUID_ASSERT(GetScalarType<Real>() == ScalarType::Real);
        LIQUID_ASSERT(GetScalarType<Integer>() == ScalarType::Integer);
        LIQUID_ASSERT(GetScalarType<Bool>() == ScalarType::Bool);

        static_assert(std::is_same_v<FromScalarType<ScalarType::Real>, Real>);
        static_assert(std::is_same_v<FromScalarType<ScalarType::Integer>, Integer>);
        static_assert(std::is_same_v<FromScalarType<ScalarType::Bool>, Bool>);

        std::cout << "done" << std::endl;
    }
}
