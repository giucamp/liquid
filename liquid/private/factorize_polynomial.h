
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "operator.h"
#include "expression.h"
#include <algorithm>

namespace liquid
{
    extern const Operator & GetOperatorAdd();
    extern const Operator & GetOperatorMul();
    extern const Operator & GetOperatorPow();

    class FactorizePolynomial
    {
    private:

        /* Given a span of integral or real scalars, return the lowest\hightest 
            value, if all elements are postive\negative.
            
            If all values are positive, returns the lowest value.
            If all values are negative, returns the highest value.
            Otherwise (even in case the span is empty) returns zero.

            This function is used to extract a factor from a pow expression
            with constant exponent. For example:
                - given pow(x, [2 3]), the factor pow(x, 2) is extracted
                - given pow(x, [-6 -5]), the factor pow(x, -5) is extracted
                - given pow(x, [-3 5]), no factor is extracted.
                
            To investigate: with a tensor factor cardinality, these constraints 
            may be removed. */
        template <typename SCALAR_TYPE>
            static SCALAR_TYPE GetExponentFloor(Span<SCALAR_TYPE const> i_scalars)
        {
            SCALAR_TYPE const zero{};
            
            if(i_scalars.empty())
                return zero;

            if(i_scalars[0] >= zero)
            {
                SCALAR_TYPE min = i_scalars[0];
                for(size_t i = 1; i < i_scalars.size(); i++)
                {
                    if(i_scalars[i] < zero)
                        return zero;
                    min = std::min(min, i_scalars[i]);
                }
                return min;
            }
            else
            {
                SCALAR_TYPE max = i_scalars[0];
                for(size_t i = 1; i < i_scalars.size(); i++)
                {
                    if(i_scalars[i] >= zero)
                        return zero;
                    max = std::max(max, i_scalars[i]);
                }
                return max;
            }
        }

        struct Factor
        {
            Tensor m_base;
            TensorValue m_exponent;
        };

        struct Monomial
        {
            std::vector<Factor> m_factors;
        };

        static std::vector<Monomial> MakePolynomial(Span<const Tensor> i_addends)
        {
            std::vector<Monomial> operand_infos;
            operand_infos.reserve(i_addends.size());
            for(const Tensor & addend : i_addends)
            {
                Monomial & operand_info = operand_infos.emplace_back();
                EnumFactors(addend, [&](const Tensor & i_base, const TensorValue & i_exponent) {

                    // search the base among the factors already present
                    bool found = false;
                    for(Factor & factor : operand_info.m_factors)
                    {
                        if(AreIdentical(factor.m_base, i_base))
                        {
                            // we just sum the two exponents
                            factor.m_exponent = GetConstantValue(
                                MakeConstant(factor.m_exponent) + MakeConstant(i_exponent));
                            found = true;
                            break;
                        }
                    }
                    if(!found)
                        operand_info.m_factors.push_back({i_base, i_exponent});
                });
            }

            return operand_infos;
        }

        /* Given a monomial, invoke the predicate for each factor composing it.
            The predicate must have this signature:
                void (const Tensor & i_base, const TensorValue & i_exponent)
            This function breaks expressions like:
                a * pow(b * c, 2 + d) * sin(a^2 * b) * a^2
            into:
                a^1, b^2, c^2, pow(b * c, d), sin(a^2 * b), a^2     */
        template <typename PREDICATE>
            static void EnumFactors(const Tensor & i_source, const PREDICATE & i_predicate)
        {
            if(i_source.GetExpression()->OperatorIs(GetOperatorMul()))
            {
                // it's a mul, recurse with operands
                for(const Tensor & operand : i_source.GetExpression()->GetOperands())
                    EnumFactors(i_source, i_predicate);
            }
            else if(i_source.GetExpression()->OperatorIs(GetOperatorPow()))
            {
                // it's a pow
                const Tensor & base = i_source.GetExpression()->GetOperand(0);
                const Tensor & exponent = i_source.GetExpression()->GetOperand(1);

                if(exponent.GetExpression()->OperatorIs(GetOperatorAdd()))
                {
                    // the exponent it's an add, recurse with base raised to every addend
                    for(const Tensor & addend : exponent.GetExpression()->GetOperands())
                        EnumFactors(Pow(base, addend), i_predicate);
                }
                else if(IsConstant(exponent))
                {
                    // constant exponent, well formed factor
                    i_predicate(base, GetConstantValue(exponent));
                }
                else
                {
                    // the exponent is a random expression
                    i_predicate(i_source, MakeConstantValue<1>());
                }
            }
            else
            {
                // not factorizable
                i_predicate(i_source, MakeConstantValue<1>());
            }
        }

    public:

        /* std::optional<Tensor> Factorize(Span<const Tensor> i_addends)
        {
            struct FactorInfo
            {
                std::vector<size_t> m_operand_indices;
            };

            std::vector<Monomial> const polynomial = MakePolynomial(i_addends);

            for(size_t monomial_index = 0; monomial_index < polynomial.size(); monomial_index++)
            {
                for(const auto & factor : polynomial[monomial_index].m_factors)
                {
                    
                }
            }
        } */

    }; // FactorizePolynomial
}
