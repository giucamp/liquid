
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6.h"
#include "miu6/parser.h"
#include "miu6/lexer.h"

namespace liquid
{
    namespace miu6
    {
        Tensor ParseExpression(std::string_view i_source)
        {
            Lexer lexer(i_source);
            Tensor const result = ParseExpression(lexer);
            
            if(!lexer.IsSourceOver())
                Panic(lexer, "expected end of source, ", 
                    GetSymbolName(lexer.GetCurrentToken().m_symbol_id), " found");
            
            return result;
        }

    } // namespace miu6
}
