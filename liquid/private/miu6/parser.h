
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "miu6/lexer.h"

namespace liquid
{
    namespace miu6
    {
        std::optional<Tensor> TryParseExpression(Lexer & i_lexer);
        Tensor ParseExpression(Lexer & i_lexer);
    }
} // namespace liquid