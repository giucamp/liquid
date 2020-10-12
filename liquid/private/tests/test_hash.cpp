
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "hash.h"
#include <iostream>

namespace liquid
{
    void TestHash()
    {
        std::cout << "Test Hash...";
        
        using namespace std::string_literals;

        LIQUID_EXPECTS(Hash(1).GetValue() == 6381476838);
        LIQUID_EXPECTS(Hash(3, 1.f).GetValue() == 7568012630088039);
        LIQUID_EXPECTS(Hash("this is a string").GetValue() == 10184142936545651121);
        LIQUID_EXPECTS(Hash("this is a string"s).GetValue() == 10184142936545651121);

        LIQUID_EXPECTS(Hash(ScalarType::Real).GetValue() == 6381476838);

        Hash hash_1("this is a string", true, 555);
        Hash hash_2 = hash_1;
        LIQUID_EXPECTS(hash_2 == hash_1);

        std::cout << "done" << std::endl;
    }
}
