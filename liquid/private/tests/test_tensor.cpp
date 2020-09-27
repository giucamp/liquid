
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
            LIQUID_ASSERT(t.HasFixedShape());
            LIQUID_ASSERT(t.GetFixedShape().content_equals({}));
            LIQUID_ASSERT(t.GetScalarType() == ScalarType::Real);
            LIQUID_ASSERT(GetConstantElements<Real>(t).content_equals({ 2.0 }));
            LIQUID_ASSERT(t + 3.0 == 5.0); 
        }

        {
            Tensor t(2.0, {3, 9});
            LIQUID_ASSERT(t.HasFixedShape());
            LIQUID_ASSERT(t.GetFixedShape().content_equals({3, 9}));
            LIQUID_ASSERT(t.GetScalarType() == ScalarType::Real);
            LIQUID_ASSERT(GetConstantElements<Real>(t).content_equals({ 2.0 }));
            LIQUID_ASSERT(t + 3.0 == 5.0);
        }

        {
            Tensor t({1, 2, 3, 4, 5, 6}, {2, 3});
            LIQUID_ASSERT(t.HasFixedShape());
            LIQUID_ASSERT(t.GetFixedShape().content_equals({2, 3}));
            LIQUID_ASSERT(t.GetScalarType() == ScalarType::Integer);
        }

        {
            
            Tensor t({ { 1., 2., 3., 4., 5., 6. }, { 6., 5., 4., 3., 2., 1. } });
            LIQUID_ASSERT(t.HasFixedShape());
            LIQUID_ASSERT(t.GetFixedShape().content_equals({ 2, 6 }));
            LIQUID_ASSERT(t.GetScalarType() == ScalarType::Real);
        }

        {
            Tensor t({1.0, 2.0});
        }

        std::cout << "done" << std::endl;
    }
}
