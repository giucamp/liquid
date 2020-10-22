
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
                    Panic("unrecognized literal type");
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
                    Panic("expected a list with a sigle element");
                return list[0];
            }
            else if (i_lexer.TryAccept(SymbolId::LeftBrace))
            {
                // scope operator {}
                std::vector<Tensor> list = ParseExpressionList(i_lexer, i_scope->MakeInner(), SymbolId::RightBrace);
                if(list.size() != 1)
                    Panic("expected a list with a sigle element");
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
                Tensor variable = MakeVariable(type, name ? name->m_source_chars: "");

                i_scope->AddVariable(variable);
                return variable;
            }

            else if (i_lexer.GetCurrentToken().IsUnaryOperator())
            {
                auto const symbol = i_lexer.GetCurrentToken().m_symbol;

                i_lexer.Advance();

                auto const applier = std::get<UnaryApplier>(symbol->m_operator_applier);
                Tensor const operand = ParseExpression(i_lexer, i_scope, symbol->m_precedence);
                return applier(operand);
            }

            // context-sensitive unary-to-binary promotion
            else if (i_lexer.TryAccept(SymbolId::BinaryMinus))
                return -ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryMinus).m_precedence);
            else if (i_lexer.TryAccept(SymbolId::BinaryPlus))
                return ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryPlus).m_precedence);

            else if (i_lexer.IsCurrentToken(SymbolId::Name))
            {
                auto const member = i_scope->TryLookup(i_lexer.GetCurrentToken().m_source_chars);
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

            while (i_lexer.GetCurrentToken().IsBinaryOperator()
                && i_lexer.GetCurrentToken().m_symbol->m_precedence >= i_min_precedence)
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
                    auto const scalar_type_opt = TryParseScalarType(i_lexer);
                    auto const shape_vector_opt = TryParseExpression(i_lexer, i_scope, 0);
                    if(!scalar_type_opt && !shape_vector_opt)
                        Panic("expected tensor type");
                    
                    ScalarType const scalar_type = scalar_type_opt ? 
                        *scalar_type_opt : ScalarType::Any;
                    TensorType::ShapeVector const shape_vector = shape_vector_opt ?
                        *shape_vector_opt : TensorType::ShapeVector{};

                    result = Is(result, TensorType(scalar_type, shape_vector));
                }
                else
                {
                    auto right = TryParseLeftExpression(i_lexer, i_scope);
                    if(!right)
                        Panic("expected right operand for ", GetSymbolChars(operator_token.m_symbol_id));

                    while (ShouldParseDeeper(i_lexer.GetCurrentToken(), operator_token))
                    {
                        right = CombineWithOperator(i_lexer, i_scope,
                            *right, i_lexer.GetCurrentToken().m_symbol->m_precedence);
                    }

                    // apply binary operand
                    auto const applier = std::get<BinaryApplier>(operator_token.m_symbol->m_operator_applier);
                    result = applier(result, *right);
                }
            }

            return result;
        }

        static bool ShouldParseDeeper(const Token & i_look_ahead, const Token & i_operator)
        {
            if(!i_look_ahead.IsBinaryOperator())
                return false;

            if(HasFlags(i_look_ahead.m_symbol->m_flags, SymbolFlags::RightAssociative))
                return i_look_ahead.m_symbol->m_precedence >= i_operator.m_symbol->m_precedence;
            else
                return i_look_ahead.m_symbol->m_precedence > i_operator.m_symbol->m_precedence;
        }

        static Tensor ParseExpression(Lexer & i_lexer, const std::shared_ptr<Scope> & i_scope, int32_t i_min_precedence)
        {
            if(auto expression = TryParseExpression(i_lexer, i_scope, i_min_precedence))
                return *expression;
            else
                Panic("expected an expression");
        }

        }; // class Parser

        Tensor ParseExpression(std::string_view i_source, const std::shared_ptr<const Scope> & i_scope)
        {
            Lexer lexer(i_source);
            try
            {
                /* prevent panic to break or log, we do it in the catch block showing
                    the current location in the source code */
                SilentPanicContext silent_panic;
                
                Tensor const result = Parser::ParseExpression(lexer, i_scope->MakeInner(), 0);

                // all the source must be consumed
                if(!lexer.IsSourceOver())
                    Panic("expected end of source, ", 
                        GetSymbolChars(lexer.GetCurrentToken().m_symbol_id), " found");

                return result;
            }
            catch(const std::exception & i_exc)
            {
                Panic(lexer, i_exc.what());
            }
            catch(...)
            {
                Panic(lexer, "unspecified error");
            }
        }
        
        /*std::shared_ptr<const Scope> ParseScope(std::string_view i_source, const std::shared_ptr<const Scope> & i_scope)
        {
            Lexer lexer(i_source);
            std::shared_ptr<const Scope> const result = Parser::ParseScope(lexer, i_scope);

            if(!lexer.IsSourceOver())
                Panic(lexer, " expected end of source, ", 
                    GetSymbolChars(lexer.GetCurrentToken().m_symbol_id), " found");

            return result;
        }*/

    } // namespace miu6

} // namespace liquid
