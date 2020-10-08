
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "miu6/lexer.h"
#include "indices.h"
#include <numeric>
#include <iostream>

namespace liquid
{
    void TestMiu6()
    {
        std::cout << "Test Miu6...";

        {
            using namespace miu6;

            Lexer lexer(" \r\n  real int\tbool   ");

            LIQUID_EXPECTS_PANIC(lexer.Accept(Token::Kind::Plus), "Unexpected token");
            
            lexer.Accept(Token::Kind::Real);
            lexer.Accept(Token::Kind::Integer);
            lexer.Accept(Token::Kind::Bool);
            lexer.Accept(Token::Kind::EndOfSource);
        }

        std::cout << "done" << std::endl;
    }
}
