
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"

namespace liquid
{
    namespace miu6
    {
        /* Every symbol in the alphabet has its own SymbolId. Anyway some SymbolId's 
            (i.e. dynamic symbols and special symbols) have no matching alphabet symbol. */
        enum class SymbolId
        {
            // scalar types
            Any, Real, Bool, Integer,

            // arithmetic unary operators
            UnaryPlus, UnaryMinus,

            // arithmetic binary operators
            BinaryPlus, BinaryMinus, Mul, Div, Pow,

            // comparison
            Equal, NotEqual, Less, Greater, LessOrEqual, GreaterOrEqual,

            // boolean operators
            And, Or, Not,

            // bracket 
            LeftParenthesis, RightParenthesis, LeftBracket, RightBracket, LeftBrace, RightBrace,

            // if
            If, Then, Elif, Else,

            // misc
            Comma, Of, Is,

            // dynamic symbols
            Name, Literal,

            // special symbols
            Unrecognized, // the lexer does not recognize a symbol
            EndOfSource, // the source code is over
        };

        enum class SymbolFlags : uint32_t
        {
            None = 0,
            BinaryOperator = 1 << 0, // binary operator
            RightAssociativeBinary = (1 << 0) | (1 << 1)  // binary operator associable from left to right
        };

        constexpr SymbolFlags operator | (SymbolFlags i_first, SymbolFlags i_second)
            { return CombineFlags(i_first, i_second); }

        // element of the alphabet 
        struct Symbol
        {
            std::string_view m_chars;
            SymbolId m_id = SymbolId::EndOfSource;
            SymbolFlags m_flags = SymbolFlags::None;
            int32_t m_precedence = -1; // precedence honored by the parser
        };

        /* The lexer recognizes symbols in the order they appear in this array.
           Shadowing must be considered: so ">" must appear after ">=", otherwise
           the former would always shadow the latter. */
        constexpr Symbol g_alphabet[] = {

            // scalar types
            { "any",        SymbolId::Any                                                           },
            { "real",       SymbolId::Real                                                          },
            { "int",        SymbolId::Integer                                                       },
            { "bool",       SymbolId::Bool                                                          },
        
            // comparison
            { "==",         SymbolId::Equal,             SymbolFlags::BinaryOperator,           100 },
            { "!=",         SymbolId::NotEqual,          SymbolFlags::BinaryOperator,           100 },
            { ">=",         SymbolId::GreaterOrEqual,    SymbolFlags::BinaryOperator,           100 },
            { "<=",         SymbolId::LessOrEqual,       SymbolFlags::BinaryOperator,           100 },
            { "<",          SymbolId::Less,              SymbolFlags::BinaryOperator,           100 },
            { ">",          SymbolId::Greater,           SymbolFlags::BinaryOperator,           100 },

            // boolean operators
            { "||",         SymbolId::Or,                SymbolFlags::BinaryOperator,           200 },
            { "&&",         SymbolId::And,               SymbolFlags::BinaryOperator,           300 },
            { "!",          SymbolId::Not,               SymbolFlags::None,                     300 },

            // arithmetic binary operators
            { "+",          SymbolId::BinaryPlus,        SymbolFlags::BinaryOperator,           500 },
            { "-",          SymbolId::BinaryMinus,       SymbolFlags::BinaryOperator,           500 },
            { "*",          SymbolId::Mul,               SymbolFlags::BinaryOperator,           600 },
            { "/",          SymbolId::Div,               SymbolFlags::BinaryOperator,           600 },
            { "^",          SymbolId::Pow,               SymbolFlags::RightAssociativeBinary,   600 },

            // unary arithmetic operators
            { "+",          SymbolId::UnaryPlus,         SymbolFlags::None,                     700 },
            { "-",          SymbolId::UnaryMinus,        SymbolFlags::None,                     700 },

            // brackets
            { "(",          SymbolId::LeftParenthesis                                               },
            { ")",          SymbolId::RightParenthesis                                              },
            { "[",          SymbolId::LeftBracket                                                   },
            { "]",          SymbolId::RightBracket                                                  },
            { "{",          SymbolId::LeftBrace                                                     },
            { "}",          SymbolId::RightBrace                                                    },

            // if
            { "if",         SymbolId::If                                                            },
            { "then",       SymbolId::Then                                                          },
            { "elif",       SymbolId::Elif                                                          },
            { "else",       SymbolId::Else                                                          },

            // misc
            { ",",          SymbolId::Comma                                                         },
            { "of ",        SymbolId::Of                                                            },
            { "is",         SymbolId::Is                                                            },
        };

        constexpr const Symbol & FindSymbol(SymbolId i_symbol_id)
        {
            for(const Symbol & symbol : g_alphabet)
                if(symbol.m_id == i_symbol_id)
                    return symbol;

            Panic("FindSymbol - unrecognized symbol id: ", static_cast<int>(i_symbol_id));
        }

        constexpr std::string_view GetSymbolName(SymbolId i_symbol_id)
        {
            for(const Symbol & symbol : g_alphabet)
                if(symbol.m_id == i_symbol_id)
                    return symbol.m_chars;

            switch(i_symbol_id)
            {
            case SymbolId::Name:
                return "<name>";

            case SymbolId::Literal:
                return "<literal>";

            case SymbolId::Unrecognized:
                return "<unrecognized>";

            case SymbolId::EndOfSource:
                return "<end_of_source>";

            default:
                Panic("GetSymbolName - unrecognized symbol id: ", static_cast<int>(i_symbol_id));
            }
            
        }

    } // namespace miu6
    
} // namespace liquid
