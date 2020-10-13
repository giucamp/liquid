
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6/parser.h"
#include "miu6/alphabet.h"
#include "tensor_type.h"
#include "expression.h"

namespace liquid
{
    namespace miu6
    {
        // https://en.wikipedia.org/wiki/Operator-precedence_parser

        class Parser { /* we use a class with static functions to avoid forward declarations
            (class members can see each other regardless of the order) */
        public:

        // parses any|real|int|bool
        static ScalarType ParseScalarType(Lexer & i_lexer)
        {
            if(i_lexer.TryAccept(SymbolId::Real))
                return ScalarType::Real;
            else if(i_lexer.TryAccept(SymbolId::Integer))
                return ScalarType::Integer;
            else if(i_lexer.TryAccept(SymbolId::Bool))
                return ScalarType::Bool;
            else if(i_lexer.TryAccept(SymbolId::Any))
                return ScalarType::Any;
            else
                return ScalarType::Any; // any is implicit
        }

        // parses any|real|int|bool [expr...,*]
        static TensorType ParseTensorType(Lexer & i_lexer)
        {
            ScalarType const scalar_type = ParseScalarType(i_lexer);
            if (i_lexer.TryAccept(SymbolId::LeftBracket))
            {
                Tensor const shape_vector = Stack(ParseExpressionList(i_lexer));
                i_lexer.Accept(SymbolId::RightBracket);
                return TensorType(scalar_type, shape_vector);
            }
            else
                return scalar_type;
        }

        // parses expr...,*
        static std::vector<Tensor> ParseExpressionList(Lexer & i_lexer)
        {
            std::vector<Tensor> result;
            if(auto first_expression = TryParseExpression(i_lexer, 0))
            {
                result.push_back(*first_expression);
                while (i_lexer.TryAccept(SymbolId::Comma))
                    result.push_back(ParseExpression(i_lexer, 0));
            }
            return result;
        }

        // tries to parse a complete expression
        static std::optional<Tensor> TryParseExpression(Lexer & i_lexer, int32_t i_min_precedence)
        {
            if (auto const left_operand = TryParseLeftExpression(i_lexer))
                return CombineWithOperator(i_lexer, *left_operand, i_min_precedence);
            else
                return {};
        }

        // tries to parse an expression that may be the left-hand-side of a binary operator
        static std::optional<Tensor> TryParseLeftExpression(Lexer & i_lexer)
        {
            if (auto token = i_lexer.TryAccept(SymbolId::Literal))
            {
                // scalar literal
                if(auto const value = std::get_if<Real>(&token->m_value))
                    return Tensor(*value);
                else if(auto const value = std::get_if<Integer>(&token->m_value))
                    return Tensor(*value);
                else if(auto const value = std::get_if<Bool>(&token->m_value))
                    return Tensor(*value);
                else
                    Panic(i_lexer, "unrecognized literal type");
            }
            else if(auto token = i_lexer.TryAccept(SymbolId::LeftBracket))
            {
                // stack operatpr - creates tensors
                std::vector<Tensor> tensors;
                while(!i_lexer.TryAccept(SymbolId::RightBracket))
                    tensors.push_back(ParseExpression(i_lexer, 0));
                return Stack(tensors);
            }
            else if(i_lexer.TryAccept(SymbolId::If))
            {
                std::vector<Tensor> operands;
                
                do {
                    // condition then value
                    operands.push_back(ParseExpression(i_lexer, 0));
                    i_lexer.Accept(SymbolId::Then);
                    operands.push_back(ParseExpression(i_lexer, 0));
                } while(i_lexer.TryAccept(SymbolId::Elif));

                // else fallback
                i_lexer.Accept(SymbolId::Else);
                operands.push_back(ParseExpression(i_lexer, 0));

                return If(operands);
            }
            else if (auto token = i_lexer.TryAccept(SymbolId::LeftParenthesis))
            {
                // parentheses
                Tensor expr = ParseExpression(i_lexer, 0);
                i_lexer.Accept(SymbolId::RightParenthesis);
                return expr;
            }
            else if(i_lexer.IsCurrentToken(SymbolId::Any) ||
                i_lexer.IsCurrentToken(SymbolId::Real) ||
                i_lexer.IsCurrentToken(SymbolId::Integer) ||
                i_lexer.IsCurrentToken(SymbolId::Bool))
            {
                // variable
                auto const type = ParseTensorType(i_lexer);
                auto const name = i_lexer.Accept(SymbolId::Name);
                return MakeVariable(type, name.m_chars);
            }
            else if (auto token = i_lexer.TryAccept(SymbolId::UnaryMinus))
                return -ParseExpression(i_lexer, FindSymbol(SymbolId::UnaryMinus).m_precedence);
            else if (auto token = i_lexer.TryAccept(SymbolId::UnaryPlus))
                return ParseExpression(i_lexer, FindSymbol(SymbolId::UnaryPlus).m_precedence);
            else if (auto token = i_lexer.TryAccept(SymbolId::BinaryMinus))
                return -ParseExpression(i_lexer, FindSymbol(SymbolId::UnaryMinus).m_precedence);
            else if (auto token = i_lexer.TryAccept(SymbolId::BinaryPlus))
                return ParseExpression(i_lexer, FindSymbol(SymbolId::UnaryPlus).m_precedence);
            else
                return {};
        }

        /* given a left operand, tries to parse a binary expression ignoring operators 
            whoose precedence is less than i_min_precedence */
        static Tensor CombineWithOperator(Lexer & i_lexer,
            const Tensor & i_left_operand, int32_t i_min_precedence)
        {
            Tensor result = i_left_operand;

            auto look_ahead = i_lexer.GetCurrentToken();
            while (HasFlags(look_ahead.m_flags, SymbolFlags::BinaryOperator)
                && look_ahead.m_precedence >= i_min_precedence)
            {
                Token operator_token = look_ahead;
                
                i_lexer.Advance();

                auto right = TryParseLeftExpression(i_lexer);
                if(!right)
                    Panic(i_lexer, "expected right operand");
                look_ahead = i_lexer.GetCurrentToken();

                while (HasFlags(look_ahead.m_flags, SymbolFlags::BinaryOperator)
                    && ShouldParseDeeper(look_ahead, operator_token))
                {
                    right = CombineWithOperator(i_lexer, *right, 
                        look_ahead.m_precedence);
                    look_ahead = i_lexer.GetCurrentToken();
                }

                result = ApplyBinaryOperator(i_lexer, result, operator_token.m_symbol_id, *right);
            }

            return result;
        }

        static Tensor ApplyBinaryOperator(Lexer & i_lexer,
            const Tensor & i_left, SymbolId i_op, const Tensor & i_right)
        {
            switch (i_op)
            {
            case SymbolId::BinaryPlus:
                return i_left + i_right;
            case SymbolId::BinaryMinus:
                return i_left - i_right;
            case SymbolId::Mul:
                return i_left * i_right;
            case SymbolId::Div:
                return i_left / i_right;
            case SymbolId::Pow:
                return Pow(i_left, i_right);
            case SymbolId::Equal:
                return i_left == i_right;
            case SymbolId::NotEqual:
                return i_left != i_right;
            case SymbolId::Less:
                return i_left < i_right;
            case SymbolId::Greater:
                return i_left > i_right;
            case SymbolId::LessOrEqual:
                return i_left <= i_right;
            case SymbolId::GreaterOrEqual:
                return i_left >= i_right;
            case SymbolId::And:
                return i_left && i_right;
            case SymbolId::Or:
                return i_left || i_right;
            default:
                Panic(i_lexer, " ApplyBinaryOperator - symbol ", GetSymbolName(i_op), " is not a binary operator");
            }
        }

        static bool ShouldParseDeeper(const Token & i_look_ahead, const Token & i_operator)
        {
            if(i_look_ahead.m_precedence < 0)
                return false;

            if(HasFlags(i_look_ahead.m_flags, SymbolFlags::RightAssociativeBinary))
                return i_look_ahead.m_precedence >= i_operator.m_precedence;
            else
                return i_look_ahead.m_precedence > i_operator.m_precedence;
        }

        static Tensor ParseExpression(Lexer & i_lexer, int32_t i_min_precedence)
        {
            if(auto expression = TryParseExpression(i_lexer, i_min_precedence))
                return *expression;
            else
                Panic(i_lexer, " expected an expression");
        }

        }; // class Parser

        std::optional<Tensor> TryParseExpression(Lexer & i_lexer)
        {
            return Parser::TryParseExpression(i_lexer, 0);
        }

        Tensor ParseExpression(Lexer & i_lexer)
        {
            return Parser::ParseExpression(i_lexer, 0);
        }

    } // namespace miu6

} // namespace liquid
