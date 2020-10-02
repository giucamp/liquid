
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
    Tensor Lerp(Tensor i_factor, Tensor i_lower, Tensor i_upper)
    {
        return If( i_factor <= 0, i_lower,
            i_factor >= 1, i_upper, 
            i_lower + (i_upper - i_lower) * i_factor );
    }

    void TestIf()
    {
        std::cout << "Test If...";

        {
            Tensor t2 = 2.0;
            Tensor t3 = 3;
            Tensor u3 = 3.0;

            LIQUID_EXPECTS(t3 > t2);
            LIQUID_EXPECTS(t2 < t3);
            LIQUID_EXPECTS(t3 >= t2);
            LIQUID_EXPECTS(t2 <= t3);
            LIQUID_EXPECTS(t2 != t3);

            LIQUID_EXPECTS(t3 == u3);
            LIQUID_EXPECTS(t3 <= u3);
            LIQUID_EXPECTS(t3 >= u3);
        }

        {
            LIQUID_EXPECTS(If(2) == 2);
            LIQUID_EXPECTS(If(true, 2, 1) == 2);
            LIQUID_EXPECTS(If(false, 2, 1.0) == 1);
            LIQUID_EXPECTS(If(true, 2, 1) == 2);
            LIQUID_EXPECTS(If(false, 2, false, 3, 1) == 1);
            LIQUID_EXPECTS(If(false, 2, false, 3, true, 1, 42) == 1);

            LIQUID_EXPECTS_ERROR(If(1, 1, 1), "If: could not find an overload matching the argument types: int[], int[], int[]");
        }

        {
            Tensor t1 = {3, 4};
            Tensor t2 = {2, 5};
            Tensor t3 = {0.0, 0.0};
            Tensor matrix = {{1, 2}, {3, 4}};

            LIQUID_EXPECTS( If(t1 > t2, 5.0, 2) == Tensor({5, 2}) );
            LIQUID_EXPECTS( If(t1 > t2 || t3 == t2, 5.0, 2) == Tensor({5, 2}) );
            LIQUID_EXPECTS( If(false, matrix, 2) == 2 );
            LIQUID_EXPECTS( If(false, matrix, 2) == Tensor({{2, 2}, {2, 2}}) );
            LIQUID_EXPECTS( If(true, matrix, 2) == matrix );

            LIQUID_EXPECTS( Lerp(0.5, 7, 8) == 7.5);
            LIQUID_EXPECTS( Lerp( {-1., 0.5, 2.}, 7, 8 ) == Tensor({7., 7.5, 8.}));
            LIQUID_EXPECTS( Lerp( 0.5, 0, {8, 2} ) == Tensor({4, 1}));
        }

        std::cout << "done" << std::endl;
    }
}
