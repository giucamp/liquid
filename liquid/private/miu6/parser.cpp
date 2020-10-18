
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6/parser.h"
#include "miu6/lexer.h"
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

        // tries to parse real|int|bool|any
        static std::optional<ScalarType> TryParseScalarType(Lexer & i_lexer)
        {
            if(i_lexer.TryAccept(SymbolId::Real))           return ScalarType::Real;
            else if(i_lexer.TryAccept(SymbolId::Integer))   return ScalarType::Integer;
            else if(i_lexer.TryAccept(SymbolId::Bool))      return ScalarType::Bool;
            else if(i_lexer.TryAccept(SymbolId::Any))       return ScalarType::Any;
            else return {};
        }

        // parses a space-separated or comma-separated list of expressions
        static std::vector<Tensor> ParseExpressionList(Lexer & i_lexer, 
            const std::shared_ptr<Scope> & i_scope, SymbolId i_terminator_symbol)
        {
            std::vector<Tensor> result;
            while(!i_lexer.TryAccept(i_terminator_symbol))
            {
                result.push_back(ParseExpression(i_lexer, i_scope, 0));
                i_lexer.TryAccept(SymbolId::Comma);
            }
            return result;
        }

        /* given an (already parsed) operator, parse a parenthesized argument list,
            or a naked single argument, possibly prefixed by "of" */
        static Tensor ParseInvokeOperator(Lexer & i_lexer, 
            const std::shared_ptr<Scope> & i_scope,
            const Operator & i_operator)
        {
            if(i_lexer.TryAccept(SymbolId::LeftParenthesis))
                return i_operator.Invoke( { ParseExpressionList(i_lexer, i_scope, SymbolId::RightParenthesis) } );

            i_lexer.TryAccept(SymbolId::Of); // 'of' is optional, meaningless, i.e. cosmetic
            return i_operator.Invoke( { ParseExpression(i_lexer, i_scope, 0) } );
        }

        // tries to parse a complete expression
        static std::optional<Tensor> TryParseExpression(Lexer & i_lexer,
            const std::shared_ptr<Scope> & i_scope, int32_t i_min_precedence)
        {
            if (auto const left_operand = TryParseLeftExpression(i_lexer, i_scope))
            {
                // try to combine with a binary operator
                auto result = CombineWithOperator(i_lexer, i_scope, 
                    *left_operand, i_min_precedence);

                return result;
            }
            else
                return {};
        }

        // tries to parse an expression that may be the left-hand-side of a binary operator
        static std::optional<Tensor> TryParseLeftExpression(Lexer & i_lexer,
            const std::shared_ptr<Scope> & i_scope)
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
            else if(i_lexer.TryAccept(SymbolId::LeftBracket))
            {
                // stack operator [] - creates a tensor
                return Stack(ParseExpressionList(i_lexer, i_scope, SymbolId::RightBracket));
            }
            else if (i_lexer.TryAccept(SymbolId::LeftParenthesis))
            {
                // list operator ()
                std::vector<Tensor> list = ParseExpressionList(i_lexer, i_scope, SymbolId::RightParenthesis);
                if(list.size() != 1)
                    Panic(i_lexer, "expected a list with a sigle element");
                return list[0];
            }
            else if (i_lexer.TryAccept(SymbolId::LeftBrace))
            {
                // scope operator {}
                std::vector<Tensor> list = ParseExpressionList(i_lexer, i_scope->MakeInner(), SymbolId::RightBrace);
                if(list.size() != 1)
                    Panic(i_lexer, "expected a list with a sigle element");
                return list[0];
            }
            else if(i_lexer.TryAccept(SymbolId::If))
            {
                std::vector<Tensor> operands;
                
                do {
                    // condition then value
                    operands.push_back(ParseExpression(i_lexer, i_scope, 0));
                    i_lexer.Accept(SymbolId::Then);
                    operands.push_back(ParseExpression(i_lexer, i_scope, 0));
                } while(i_lexer.TryAccept(SymbolId::Elif));

                // else fallback
                i_lexer.Accept(SymbolId::Else);
                operands.push_back(ParseExpression(i_lexer, i_scope, 0));

                return If(operands);
            }
            else if(auto const scalar_type = TryParseScalarType(i_lexer))
            {
                // variable declaration: both shape and name are optional
                auto const shape_vector = TryParseExpression(i_lexer, i_scope, 0);
                auto const name = i_lexer.TryAccept(SymbolId::Name);

                // if shape_vector is non-null but not a vector, the following will panic...
                TensorType const type = shape_vector ? TensorType{*scalar_type, *shape_vector} : *scalar_type;
                Tensor variable = MakeVariable(type, name ? name->m_chars : "");

                i_scope->AddVariable(variable);
                return variable;
            }

            else if (i_lexer.TryAccept(SymbolId::Not))
            {
                // not
                return !ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::Not).m_precedence);
            }

            // unary plus\minus
            else if (i_lexer.TryAccept(SymbolId::UnaryMinus))
                return -ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryMinus).m_precedence);
            else if (i_lexer.TryAccept(SymbolId::UnaryPlus))
                return ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryPlus).m_precedence);

            // context-sensitive unary-to-binary promotion
            else if (i_lexer.TryAccept(SymbolId::BinaryMinus))
                return -ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryMinus).m_precedence);
            else if (i_lexer.TryAccept(SymbolId::BinaryPlus))
                return ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryPlus).m_precedence);

            else if (i_lexer.IsCurrentToken(SymbolId::Name))
            {
                auto const member = i_scope->TryLookup(i_lexer.GetCurrentToken().m_chars);
                if(std::holds_alternative<std::reference_wrapper<const Operator>>(member))
                {
                    i_lexer.Advance();
                    return ParseInvokeOperator(i_lexer, i_scope, 
                        std::get<std::reference_wrapper<const Operator>>(member).get());
                }
                else if(std::holds_alternative<Tensor>(member))
                {
                    i_lexer.Advance();
                    return std::get<Tensor>(member);
                }
                else
                    return {};
            }

            return {};
        }

        /* given a left operand, tries to parse a binary expression ignoring operators 
            whoose precedence is less than i_min_precedence */
        static Tensor CombineWithOperator(Lexer & i_lexer, const std::shared_ptr<Scope> & i_scope,
            const Tensor & i_left_operand, int32_t i_min_precedence)
        {
            Tensor result = i_left_operand;

            while (HasFlags(i_lexer.GetCurrentToken().m_flags, SymbolFlags::BinaryOperator)
                && i_lexer.GetCurrentToken().m_precedence >= i_min_precedence)
            {
                /* now we have the left-hand operatand and the operator.
                   we just need the right-hand operand. */
                Token const operator_token = i_lexer.GetCurrentToken();

                // we have accepted the operator, so we must move to the lext token
                i_lexer.Advance();

                if(operator_token.m_symbol_id == SymbolId::Is)
                {
                    /* 'is' is special because the second operand is a type.
                       Clinton's law: "it depends on what the meaning of 'is' is" */
                    auto scalar_type = TryParseScalarType(i_lexer);
                    if(!scalar_type)
                        Panic(i_lexer, " expected scalar type");
                    
                    result = Is(result, *scalar_type);
                }
                else
                {
                    auto right = TryParseLeftExpression(i_lexer, i_scope);
                    if(!right)
                        Panic(i_lexer, " expected right operand for ", GetSymbolName(operator_token.m_symbol_id));

                    while (ShouldParseDeeper(i_lexer.GetCurrentToken(), operator_token))
                    {
                        right = CombineWithOperator(i_lexer, i_scope,
                            *right, i_lexer.GetCurrentToken().m_precedence);
                    }

                    result = ApplyBinaryOperator(i_lexer, result, 
                        operator_token.m_symbol_id, *right);
                }
            }

            return result;
        }

        static bool ShouldParseDeeper(const Token & i_look_ahead, const Token & i_operator)
        {
            if(!HasFlags(i_look_ahead.m_flags, SymbolFlags::BinaryOperator))
                return false;

            if(HasFlags(i_look_ahead.m_flags, SymbolFlags::RightAssociativeBinary))
                return i_look_ahead.m_precedence >= i_operator.m_precedence;
            else
                return i_look_ahead.m_precedence > i_operator.m_precedence;
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

        static Tensor ParseExpression(Lexer & i_lexer, const std::shared_ptr<Scope> & i_scope, int32_t i_min_precedence)
        {
            if(auto expression = TryParseExpression(i_lexer, i_scope, i_min_precedence))
                return *expression;
            else
                Panic(i_lexer, " expected an expression");
        }

        }; // class Parser

        Tensor ParseExpression(std::string_view i_source, const std::shared_ptr<const Scope> & i_scope)
        {
            Lexer lexer(i_source);
            Tensor const result = Parser::ParseExpression(lexer, i_scope->MakeInner(), 0);

            if(!lexer.IsSourceOver())
                Panic(lexer, " expected end of source, ", 
                    GetSymbolName(lexer.GetCurrentToken().m_symbol_id), " found");

            return result;
        }
        
        /*std::shared_ptr<const Scope> ParseScope(std::string_view i_source, const std::shared_ptr<const Scope> & i_scope)
        {
            Lexer lexer(i_source);
            std::shared_ptr<const Scope> const result = Parser::ParseScope(lexer, i_scope);

            if(!lexer.IsSourceOver())
                Panic(lexer, " expected end of source, ", 
                    GetSymbolName(lexer.GetCurrentToken().m_symbol_id), " found");

            return result;
        }*/

    } // namespace miu6

} // namespace liquid
