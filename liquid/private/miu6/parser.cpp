
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6/parser.h"
#include "tensor_type.h"

namespace liquid
{
    namespace miu6
    {
        std::optional<Tensor> TryParseExpression(Lexer & i_lexer);
        Tensor ParseExpression(Lexer & i_lexer);
        std::vector<Tensor> ParseExpressionList(Lexer & i_lexer);
        ScalarType ParseScalarType(Lexer & i_lexer);
        TensorType ParseTensorType(Lexer & i_lexer);

        /** returns wheter an operator has left-to-right associativity */
        std::optional<Tensor> TryParseLeftExpression(Lexer & i_lexer);
        Tensor CombineWithOperator(Lexer & i_lexer, const Tensor & i_left_operand, int32_t i_min_precedence);
        bool IsOperatorLeftAssociative(Token::Kind i_token_kind);
        Tensor ApplyBinaryOperator(Token::Kind i_op, const Tensor & i_left, const Tensor & i_right);
        int32_t GetBinaryOperatorPrecedence(Token::Kind i_token_kind);
        bool ParseDeeper(Lexer & i_lexer, const Token & i_operator_token);

        ScalarType ParseScalarType(Lexer & i_lexer)
        {
            if(i_lexer.TryAccept(Token::Kind::Real))
                return ScalarType::Real;
            else if(i_lexer.TryAccept(Token::Kind::Integer))
                return ScalarType::Integer;
            else if(i_lexer.TryAccept(Token::Kind::Bool))
                return ScalarType::Bool;
            else if(i_lexer.TryAccept(Token::Kind::Bool))
                return ScalarType::Any;
            else
                return ScalarType::Any; // any is implicit
        }

        TensorType ParseTensorType(Lexer & i_lexer)
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

        std::vector<Tensor> ParseExpressionList(Lexer & i_lexer)
        {
            std::vector<Tensor> result;
            if(auto first_expression = TryParseExpression(i_lexer))
            {
                result.push_back(*first_expression);
                while (i_lexer.TryAccept(Token::Kind::Comma))
                    result.push_back(ParseExpression(i_lexer));
            }
            return result;
        }

        std::optional<Tensor> TryParseLeftExpression(Lexer & i_lexer)
        {
            if (auto token = i_lexer.TryAccept(Token::Kind::Literal))
            {
                // literal
                if(std::holds_alternative<Real>(token->m_value))
                    return Tensor(std::any_cast<Real>(token->m_value));
                else if(std::holds_alternative<Integer>(token->m_value))
                    return Tensor(std::any_cast<Integer>(token->m_value));
                else if(std::holds_alternative<Bool>(token->m_value))
                    return Tensor(std::any_cast<Bool>(token->m_value));
                else
                    Panic("unrecognized literal type");
            }
            else if (auto token = i_lexer.TryAccept(Token::Kind::LeftParenthesis))
            {
                // parentheses
                Tensor expr = ParseExpression(i_lexer);
                i_lexer.Accept(Token::Kind::RightParenthesis);
                return expr;
            }
            else if (auto token = i_lexer.TryAccept(Token::Kind::Minus))
                return -ParseExpression(i_lexer); // unary minus
            else if (auto token = i_lexer.TryAccept(Token::Kind::Plus))
                return ParseExpression(i_lexer); // unary plus
            else
                return {};
        }

        std::optional<Tensor> TryParseExpression(Lexer & i_lexer)
        {
            if (auto const left_operand = TryParseLeftExpression(i_lexer))
                return CombineWithOperator(i_lexer, *left_operand, 0);
            else
                return {};
        }

        Tensor ApplyBinaryOperator(Token::Kind i_op, const Tensor & i_left, const Tensor & i_right)
        {
            switch (i_op)
            {
            case liquid::miu6::Token::Kind::Plus:
                return i_left + i_right;
            case liquid::miu6::Token::Kind::Minus:
                return i_left - i_right;
            case liquid::miu6::Token::Kind::Times:
                return i_left * i_right;
            case liquid::miu6::Token::Kind::Div:
                return i_left / i_right;
            case liquid::miu6::Token::Kind::Pow:
                return Pow(i_left, i_right);
            case liquid::miu6::Token::Kind::Equal:
                return i_left == i_right;
            case liquid::miu6::Token::Kind::NotEqual:
                return i_left != i_right;
            case liquid::miu6::Token::Kind::Less:
                return i_left < i_right;
            case liquid::miu6::Token::Kind::Greater:
                return i_left > i_right;
            case liquid::miu6::Token::Kind::LessOrEqual:
                return i_left <= i_right;
            case liquid::miu6::Token::Kind::GreaterOrEqual:
                return i_left >= i_right;
            case liquid::miu6::Token::Kind::And:
                return i_left && i_right;
            case liquid::miu6::Token::Kind::Or:
                return i_left || i_right;
            default:
                Panic("ApplyBinaryOperator - token ", static_cast<int>(i_op), " is a binary operator");
            }
        }

        bool ParseDeeper(Lexer & i_lexer, const Token & i_operator_token)
        {
            int32_t op_precedence = GetBinaryOperatorPrecedence(i_operator_token.m_kind);
            int32_t curr_precedence = GetBinaryOperatorPrecedence(i_lexer.GetCurrentToken().m_kind);
            if(!IsOperatorLeftAssociative(i_lexer.GetCurrentToken().m_kind))
                curr_precedence++;

            return curr_precedence > op_precedence;
        }

        Tensor CombineWithOperator(Lexer & i_lexer, const Tensor & i_left_operand, int32_t i_min_precedence)
        {
            Tensor result = i_left_operand;

            while (GetBinaryOperatorPrecedence(i_lexer.GetCurrentToken().m_kind) >= i_min_precedence)
            {
                Token const operator_token = i_lexer.GetCurrentToken();
                i_lexer.Advance();
                Tensor right = ParseExpression(i_lexer);

                while (ParseDeeper(i_lexer, operator_token))
                {
                    right = CombineWithOperator(i_lexer, i_left_operand, 
                        GetBinaryOperatorPrecedence(i_lexer.GetCurrentToken().m_kind));
                }

                result = ApplyBinaryOperator(operator_token.m_kind, result, right);
            }

            return result;
        }

        Tensor ParseExpression(Lexer & i_lexer)
        {
            if(auto expression = TryParseExpression(i_lexer))
                return *expression;
            else
                Panic("expected an expression");
        }

        bool IsOperatorLeftAssociative(Token::Kind i_token_kind)
        {
            return i_token_kind == Token::Kind::Pow;
        }

        int32_t GetBinaryOperatorPrecedence(Token::Kind i_token_kind)
        {
            switch (i_token_kind)
            {
            case liquid::miu6::Token::Kind::Equal:
            case liquid::miu6::Token::Kind::Less:
            case liquid::miu6::Token::Kind::LessOrEqual:
            case liquid::miu6::Token::Kind::Greater:
            case liquid::miu6::Token::Kind::GreaterOrEqual:
                return 100;

            case liquid::miu6::Token::Kind::Plus:
            case liquid::miu6::Token::Kind::Minus:
                return 200;

            case liquid::miu6::Token::Kind::Times:
            case liquid::miu6::Token::Kind::Div:
                return 300;

            case liquid::miu6::Token::Kind::Pow:
                return 400;

            default:
                Panic("GetBinaryOperatorPrecedence - token ", static_cast<int>(i_token_kind), 
                    " is a binary operator");
            }
        }

    } // namespace miu6

} // namespace liquid
