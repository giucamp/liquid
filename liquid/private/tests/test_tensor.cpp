
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
    void TestTensor()
    {
        std::cout << "Test Tensor...";

        TensorType nnn(ScalarType::Real, FixedShape({2, 3, 2}));

        {
            Tensor t = 2.0;
            LIQUID_EXPECTS(( Rank(t) == 0 ));
            LIQUID_EXPECTS(( Shape(t) == Tensor(std::initializer_list<Tensor>{}, {0}) ));
            LIQUID_EXPECTS(( t.GetScalarType() == ScalarType::Real ));
            LIQUID_EXPECTS(( GetConstantStorage<Real>(t).content_equals({ 2.0 }) ));

            LIQUID_EXPECTS(( t + 3.0 == 5.0 )); 
        }

        {
            Tensor t1 = 1;
            Tensor t2 = 2;
            Tensor t = {t1, t2};
            Tensor t11 = {{t1, t2}, {t1, t2}};
        }

        {
            Tensor t(2.0, {3, 9});
            LIQUID_EXPECTS(( Rank(t) == 2 ));
            LIQUID_EXPECTS(( Shape(t) == Tensor({3, 9}) ));
            LIQUID_EXPECTS(( t.GetScalarType() == ScalarType::Real ));
            LIQUID_EXPECTS(( GetConstantStorage<Real>(t).content_equals({ 2.0 }) ));
            LIQUID_EXPECTS(( t + 3.0 == 5.0 ));
        }

        {
            Tensor t({1, 2, 3, 4, 5, 6}, {2, 3});
            LIQUID_EXPECTS(( Rank(t) == 2 ));
            LIQUID_EXPECTS(( Shape(t) == Tensor({2, 3}) ));
            LIQUID_EXPECTS(t.GetScalarType() == ScalarType::Integer);
        }

        {
            Tensor t({{ 1, 2, 3, 4, 5, 6 }, { 1, 2, 3, 4, 5, 6 }, { 1, 2, 3, 4, 5, 6 } });
            LIQUID_EXPECTS(( GetConstantStorage<Integer>(t).content_equals({ 1, 2, 3, 4, 5, 6 }) ));
            LIQUID_EXPECTS((Rank(t) == 2));
            LIQUID_EXPECTS((Shape(t) == Tensor({ 3, 6 })));
            LIQUID_EXPECTS(t.GetScalarType() == ScalarType::Integer);
        }

        {
            
            Tensor t({ { 1., 2., 3., 4., 5., 6. }, { 6., 5., 4., 3., 2., 1. } });
            LIQUID_EXPECTS(( Rank(t) == 2 ));
            LIQUID_EXPECTS(( Shape(t) == Tensor({2, 6}) ));
            LIQUID_EXPECTS(t.GetScalarType() == ScalarType::Real);
        }

        {
            Tensor t({1.0, 2.0});

            LIQUID_EXPECTS(( Add() == 0 ));
            LIQUID_EXPECTS(( Add(t) == Tensor({1, 2}) ));
            LIQUID_EXPECTS(( Add(t, t) == t * 2 ));

            LIQUID_EXPECTS_PANIC(Add(true), "add: could not find an overload matching the argument types: bool[]");
        }

        {
            Expects("[1 2] is int[2]");
            Expects("[ log[ 3 real[] ]"
                    "  exp[5 6] + 2  ] is real [2, 2]");
        }

        std::cout << "done" << std::endl;
    }
}
