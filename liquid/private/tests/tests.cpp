
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"

namespace liquid
{
    void TestIndices();
    void TestFixedShape();
    void TestScalarType();
    void TestTensor();
    void TestAdd();
    void TestIf();
    void TestSubstutute();
    void TestMiu6();

    void TestLiquid()
    {
        TestIndices();
        TestFixedShape();
        TestScalarType();
        TestTensor();
        TestAdd();
        TestIf();
        TestSubstutute();
        TestMiu6();
    }
}
