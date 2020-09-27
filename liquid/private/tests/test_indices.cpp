
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
    void TestIndices()
    {
        std::cout << "TestIndices...";

        FixedShape const shape{ 3, 4, 6 };
        Real values[3][4][6];

        Indices indices(shape);
        Integer linear_index = 0;
        LIQUID_ASSERT(shape.GetLinearSize() == 3 * 4 * 6);
        for (Integer i = 0; i < 6; i++)
        {
            for (Integer j = 0; j < 4; j++)
            {
                for (Integer k = 0; k < 3; k++)
                {
                    Integer n = k + j * 3 + i * 3 * 4;
                    LIQUID_ASSERT(n == linear_index);
                    LIQUID_ASSERT(n == indices.GetLogicalLinearIndex());
                    values[k][j][i] = static_cast<Real>(k + j + i);
                    indices++;
                    linear_index++;
                }
            }
        }

        linear_index = 0;
        SharedArray<Real> check_values(static_cast<size_t>(shape.GetLinearSize()));
        for(indices = Indices(shape); indices; indices++)
        {
            auto sum = std::accumulate(indices.GetIndices().begin(), indices.GetIndices().end(), 0.);
            indices[check_values] = static_cast<Real>(sum);
            linear_index++;
        }
        for (indices = Indices(shape); indices; indices++)
        {
            auto ind = indices.GetIndices();
            auto first = values[ind[0]][ind[1]][ind[2]];
            auto second = indices[check_values];
            LIQUID_ASSERT(first == second);
        }
        LIQUID_ASSERT(linear_index == 72);

        LIQUID_ASSERT(GetScalarType<Real>() == ScalarType::Real);

        std::cout << "done" << std::endl;
    }
}
