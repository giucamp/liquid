
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "expression.h"
#include "substitute_by_predicate.h"
#include <numeric>
#include <iostream>

namespace liquid
{
    void TestSubstutute()
    {
        std::cout << "Test Substutute...";

 
        {
            auto tensor = SubstituteByPredicate(Tensor("real x + 2"), [](const Tensor & i_candidate){
                if(i_candidate.GetExpression()->GetName() == "x")
                    return Tensor(4);
                else
                    return i_candidate;
            });

            LIQUID_EXPECTS(tensor == 6);
        }


        std::cout << "done" << std::endl;
    }
}
