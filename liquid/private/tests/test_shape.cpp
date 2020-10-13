
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "indices.h"
#include "book.h"
#include <numeric>
#include <iostream>

namespace liquid
{
    void TestFixedShape()
    {
        std::cout << "Test FixedShape...";

        const char topic[] = "shape broadcasting";

        Book::Get().AddParagraph(topic,
            "Broadcasting is the extension of the shape of a tensor along"
            " its one or unesisting dimensions to match the shape of another"
            " tensor.\n"
            "For example when a matrix is multiplied by a scalar, the scalar"
            " is replicated along rows and columns to form a matrix: \n"
            "[[ *, *, *],           [[ *, *, *],   [[ x, x, x],"
            " [ *, *, *], * x  -->   [ *, *, *], *  [ x, x, x],"
            " [ *, *, *]]            [ *, *, *]]    [ x, x, x]]"
            
        );

        LIQUID_EXPECTS_DOC(topic, Shape({4, 5}) == Tensor({2}));

        std::cout << "done" << std::endl;
    }
}
