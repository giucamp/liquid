﻿
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

            Lexer lexer(" \r\n  real int\t+bool a + b ");

            LIQUID_EXPECTS_PANIC(lexer.Accept(SymbolId::BinaryMinus), "Unexpected token");
            
            lexer.Accept(SymbolId::Real);
            lexer.Accept(SymbolId::Integer);
            lexer.Accept(SymbolId::UnaryPlus);
            lexer.Accept(SymbolId::Bool);
            lexer.Accept(SymbolId::Name);
            lexer.Accept(SymbolId::BinaryPlus);
            lexer.Accept(SymbolId::Name);
            lexer.Accept(SymbolId::EndOfSource);
        }

        Expects("2 + 3*5 == 17");
        Expects("2 + 3*5*(2+3) == 77");
        Expects("(2+-3)*2 + 40 == 38");
        Expects("4^3^2+1==262145");
        Expects("2+3*3>0");
        Expects("(2+3*3)*0>=0");        
        Expects("-5==-(5)");        
        Expects("40/5*2 == 16");
        Expects("-3^2==9");
        Expects("(2+-3^2)==11");
        Expects("-100---50    ==    (-(2+-3^2^((2)))*2 + 40/(((5)))*2)");
        Expects("-(2+-3^2)*2 + 40/5*2 == -6");

        Expects("[1 -2] == [(1) (1-3)]");

        Expects("[1 2] * 2 == [2 4]");
        Expects("[[1 2][3 4]] * 2 > 0");
        Expects("[[1 3+2][3 4]] * 2 == [[2 10][6 8]]");

        Expects("real x * 0 * real y == 0"); 

        Expects("real * 0 * real == 0"); 

        Expects(u8"[[ cos real[] θ     -sin θ ] "
                u8" [ sin θ            cos θ  ]] is real[2 2]");

        // to do: implement softmax
        // https://timvieira.github.io/blog/post/2014/02/11/exp-normalize-trick/

        std::cout << "done" << std::endl;
    }
}
