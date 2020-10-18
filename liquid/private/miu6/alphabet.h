
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include <variant>

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
            RightAssociative = (1 << 0) // binary operator associable from left to right
        };

        constexpr SymbolFlags operator | (SymbolFlags i_first, SymbolFlags i_second)
            { return CombineFlags(i_first, i_second); }

        using BinaryApplier = Tensor (*)(const Tensor & i_left, const Tensor & i_right);
        using UnaryApplier = Tensor (*)(const Tensor & i_operand);
        using OperatorApplier = std::variant<std::monostate, UnaryApplier, BinaryApplier>;

        // element of the alphabet 
        struct Symbol
        {
            std::string_view m_chars;
            SymbolId m_id = SymbolId::EndOfSource;
            OperatorApplier m_operator_applier;
            int32_t m_precedence = -1; // precedence honored by the parser
            SymbolFlags m_flags = SymbolFlags::None;

            constexpr bool IsUnaryOperator() const
                { return std::holds_alternative<UnaryApplier>(m_operator_applier); }

            constexpr bool IsBinaryOperator() const
                { return std::holds_alternative<BinaryApplier>(m_operator_applier); }
        };

        /* The lexer recognizes symbols in the order they appear in this array.
           Shadowing must be considered: so ">" must appear after ">=", otherwise
           the former would always shadow the latter. */
        constexpr Symbol g_alphabet[] = {

            // scalar types
            { "any",        SymbolId::Any                                    },
            { "real",       SymbolId::Real                                   },
            { "int",        SymbolId::Integer                                },
            { "bool",       SymbolId::Bool                                   },
        
            // comparison
            { "==",         SymbolId::Equal,             operator ==,     100 },
            { "!=",         SymbolId::NotEqual,          operator !=,     100 },
            { ">=",         SymbolId::GreaterOrEqual,    operator >=,     100 },
            { "<=",         SymbolId::LessOrEqual,       operator <=,     100 },
            { ">",          SymbolId::Greater,           operator >,      100 },
            { "<",          SymbolId::Less,              operator <,      100 },
            
            // boolean operators
            { "or",         SymbolId::Or,                operator ||,     200 },
            { "and",        SymbolId::And,               operator &&,     300 },
            { "not",        SymbolId::Not,               operator !,      300 },

            // arithmetic binary operators
            { "+",          SymbolId::BinaryPlus,        (BinaryApplier)operator +,     500 },
            { "-",          SymbolId::BinaryMinus,       (BinaryApplier)operator -,     500 },
            { "*",          SymbolId::Mul,               operator *,                    600 },
            { "/",          SymbolId::Div,               operator /,                    600 },
            { "^",          SymbolId::Pow,               Pow,                           600, SymbolFlags::RightAssociative },

            // unary arithmetic operators
            { "+",          SymbolId::UnaryPlus,         (UnaryApplier)operator +,      700 },
            { "-",          SymbolId::UnaryMinus,        (UnaryApplier)operator -,      700 },

            // brackets
            { "(",          SymbolId::LeftParenthesis                                       },
            { ")",          SymbolId::RightParenthesis                                      },
            { "[",          SymbolId::LeftBracket                                           },
            { "]",          SymbolId::RightBracket                                          },
            { "{",          SymbolId::LeftBrace                                             },
            { "}",          SymbolId::RightBrace                                            },

            // if
            { "if",         SymbolId::If                                                    },
            { "then",       SymbolId::Then                                                  },
            { "elif",       SymbolId::Elif                                                  },
            { "else",       SymbolId::Else                                                  },

            // misc
            { ",",          SymbolId::Comma                                                 },
            { "of ",        SymbolId::Of                                                    },
            { "is",         SymbolId::Is,                BinaryApplier{},               400 },
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
