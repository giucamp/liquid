
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6/parser.h"
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
            if(i_lexer.TryAccept(Token::Kind::Real))
                return ScalarType::Real;
            else if(i_lexer.TryAccept(Token::Kind::Integer))
                return ScalarType::Integer;
            else if(i_lexer.TryAccept(Token::Kind::Bool))
                return ScalarType::Bool;
            else if(i_lexer.TryAccept(Token::Kind::Any))
                return ScalarType::Any;
            else
                return ScalarType::Any; // any is implicit
        }

        // parses any|real|int|bool [expr...,*]
        static TensorType ParseTensorType(Lexer & i_lexer)
        {
            ScalarType const scalar_type = ParseScalarType(i_lexer);
            if (i_lexer.TryAccept(Token::Kind::LeftBracket))
            {
                Tensor const shape_vector = Stack(ParseExpressionList(i_lexer));
                i_lexer.Accept(Token::Kind::RightBracket);
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
                while (i_lexer.TryAccept(Token::Kind::Comma))
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
            if (auto token = i_lexer.TryAccept(Token::Kind::Literal))
            {
                // scalar literal
                if(auto const value = std::get_if<Real>(&token->m_value))
                    return Tensor(*value);
                else if(auto const value = std::get_if<Integer>(&token->m_value))
                    return Tensor(*value);
                else if(auto const value = std::get_if<Bool>(&token->m_value))
                    return Tensor(*value);
                else
                    Panic("unrecognized literal type");
            }
            else if(auto token = i_lexer.TryAccept(Token::Kind::LeftBracket))
            {
                // tensor
                std::vector<Tensor> tensors;
                while(!i_lexer.TryAccept(Token::Kind::RightBracket))
                    tensors.push_back(ParseExpression(i_lexer, 0));
                return Stack(tensors);
            }
            else if (auto token = i_lexer.TryAccept(Token::Kind::LeftParenthesis))
            {
                // parentheses
                Tensor expr = ParseExpression(i_lexer, 0);
                i_lexer.Accept(Token::Kind::RightParenthesis);
                return expr;
            }
            else if(i_lexer.IsCurrentToken(Token::Kind::Any) ||
                i_lexer.IsCurrentToken(Token::Kind::Real) ||
                i_lexer.IsCurrentToken(Token::Kind::Integer) ||
                i_lexer.IsCurrentToken(Token::Kind::Bool))
            {
                // variable
                auto const type = ParseTensorType(i_lexer);
                auto const name = i_lexer.Accept(Token::Kind::Name);
                return MakeVariable(type, name.m_chars);
            }
            else if (auto token = i_lexer.TryAccept(Token::Kind::Minus))
                return -ParseExpression(i_lexer, 500); // unary minus prcedence is 500
            else if (auto token = i_lexer.TryAccept(Token::Kind::Plus))
                return ParseExpression(i_lexer, 500); // unary plus prcedence is 500
            else
                return {};
        }

        /* given a left operand, tries to parse a binary expression ignoring operators 
            whoose precedence is less than i_min_precedence */
        static Tensor CombineWithOperator(Lexer & i_lexer, const Tensor & i_left_operand, int32_t i_min_precedence)
        {
            Tensor result = i_left_operand;

            auto look_ahead = i_lexer.GetCurrentToken();
            while (GetBinaryOperatorPrecedence(look_ahead.m_kind) >= i_min_precedence)
            {
                Token operator_token = look_ahead;
                
                i_lexer.Advance();
                
                auto right = *TryParseLeftExpression(i_lexer);
                
                look_ahead = i_lexer.GetCurrentToken();

                while (ShouldParseDeeper(look_ahead, operator_token))
                {
                    right = CombineWithOperator(i_lexer, right, 
                        GetBinaryOperatorPrecedence(look_ahead.m_kind));
                    look_ahead = i_lexer.GetCurrentToken();
                }

                result = ApplyBinaryOperator(result, operator_token.m_kind, right);
            }

            return result;
        }

        static Tensor ApplyBinaryOperator(const Tensor & i_left, Token::Kind i_op, const Tensor & i_right)
        {
            switch (i_op)
            {
            case Token::Kind::Plus:
                return i_left + i_right;
            case Token::Kind::Minus:
                return i_left - i_right;
            case Token::Kind::Times:
                return i_left * i_right;
            case Token::Kind::Div:
                return i_left / i_right;
            case Token::Kind::Pow:
                return Pow(i_left, i_right);
            case Token::Kind::Equal:
                return i_left == i_right;
            case Token::Kind::NotEqual:
                return i_left != i_right;
            case Token::Kind::Less:
                return i_left < i_right;
            case Token::Kind::Greater:
                return i_left > i_right;
            case Token::Kind::LessOrEqual:
                return i_left <= i_right;
            case Token::Kind::GreaterOrEqual:
                return i_left >= i_right;
            case Token::Kind::And:
                return i_left && i_right;
            case Token::Kind::Or:
                return i_left || i_right;
            default:
                Panic("ApplyBinaryOperator - token ", static_cast<int>(i_op), " is not a binary operator");
            }
        }

        static bool ShouldParseDeeper(const Token & i_look_ahead, const Token & i_operator)
        {
            int32_t const look_ahead_precedence = GetBinaryOperatorPrecedence(i_look_ahead.m_kind);
            int32_t const op_precedence = GetBinaryOperatorPrecedence(i_operator.m_kind);
            if(look_ahead_precedence < 0)
                return false;

            if(IsOperatorRightAssociative(i_look_ahead.m_kind))
                return look_ahead_precedence >= op_precedence;
            else
                return look_ahead_precedence > op_precedence;
        }

        static Tensor ParseExpression(Lexer & i_lexer, int32_t i_min_precedence)
        {
            if(auto expression = TryParseExpression(i_lexer, i_min_precedence))
                return *expression;
            else
                Panic("expected an expression");
        }

        /** returns wheter an operator has left-to-right associativity */
        static bool IsOperatorRightAssociative(Token::Kind i_token_kind)
        {
            return i_token_kind == Token::Kind::Pow;
        }

        static int32_t GetBinaryOperatorPrecedence(Token::Kind i_token_kind)
        {
            switch (i_token_kind)
            {
            case Token::Kind::Equal:
            case Token::Kind::NotEqual:
            case Token::Kind::Less:
            case Token::Kind::LessOrEqual:
            case Token::Kind::Greater:
            case Token::Kind::GreaterOrEqual:
                return 100;

            case Token::Kind::Plus:
            case Token::Kind::Minus:
                return 200;

            case Token::Kind::Times:
            case Token::Kind::Div:
                return 300;

            case Token::Kind::Pow:
                return 400;

            default:
                return -1000;
            }
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
