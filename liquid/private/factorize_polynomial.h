
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

        struct Factor
        {
            Tensor i_base;
            TensorValue i_exponent;
        };

        struct Monomial
        {
            std::vector<Factor> m_factors;
        };

        static std::vector<Monomial> CanonicalizePolynomial(Span<const Tensor> i_addends)
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
                        if(AreIdentical(factor.i_base, i_base))
                        {
                            // we just sum the two exponents
                            factor.i_exponent = GetConstantValue(MakeConstant(factor.i_exponent) + MakeConstant(i_exponent));
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
                    // the exponent is any expression
                    i_predicate(i_source, TensorValue::GetConstant<1>());
                }
            }
            else
            {
                // not factorizable
                i_predicate(i_source, TensorValue::GetConstant<1>());
            }
        }

    }; // FactorizePolynomial
}
