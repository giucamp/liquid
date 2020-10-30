
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "operator.h"
#include "expression.h"
#include "indices.h"
#include <algorithm>
#include <variant>

namespace liquid
{
    extern const Operator & GetOperatorAdd();
    extern const Operator & GetOperatorMul();
    extern const Operator & GetOperatorPow();

    namespace
    {
        /* Given a monomial, invoke the predicate for each factor composing it.
            The predicate must have this signature:
                void (const Tensor & i_base, const TensorValue & i_exponent)
            This function breaks expressions like:
                a * pow(b * c, 2 + d) * sin(a^2 * b) * a^2
            into:
                a^1, b^2, c^2, pow(b * c, d)^1, sin(a^2 * b)^1, a^2     */
        template <typename PREDICATE>
            void EnumFactors(const Tensor & i_source, const PREDICATE & i_predicate)
        {
            if(i_source.GetExpression()->OperatorIs(GetOperatorMul()))
            {
                // it's a mul, recurse with operands
                for(const Tensor & operand : i_source.GetExpression()->GetOperands())
                    EnumFactors(operand, i_predicate);
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

        struct Factor
        {
            Tensor m_base;
            std::variant<std::monostate, SharedArray<Real>, SharedArray<Integer>> m_exponent;
            FixedShape m_exponent_shape;
            std::vector<size_t> m_term_indices;

            ScalarType GetExponentType() const
            {
                if(std::holds_alternative<SharedArray<Real>>(m_exponent))
                    return ScalarType::Real;
                else
                    return ScalarType::Integer;
            }

            Factor(const Tensor & i_base, const TensorValue & i_exponent, size_t i_first_term_index)
                : m_base(i_base), m_exponent_shape(i_exponent.GetShape())
            {
                if(i_exponent.GetScalarType() == ScalarType::Real)
                    m_exponent = SharedArray<Real>(i_exponent.GetAs<Real>());
                else if(i_exponent.GetScalarType() == ScalarType::Integer)
                    m_exponent = SharedArray<Integer>(i_exponent.GetAs<Integer>());
                else
                    Panic("Factor - unexpected exponent type: ", i_exponent.GetScalarType());

                m_term_indices.push_back(i_first_term_index);
            }

            TensorValue GetAndResetExponent()
            {
                auto const scalar_type = GetExponentType();
                if(scalar_type == ScalarType::Real)
                    return TensorValue(std::move(std::get<SharedArray<Real>>(m_exponent)), m_exponent_shape);
                else if(scalar_type == ScalarType::Integer)
                    return TensorValue(std::move(std::get<SharedArray<Integer>>(m_exponent)), m_exponent_shape);
                else
                    Panic("GetAndResetExponent - unexpected exponent type");
            }

            /* converts m_exponent from integer to real if i_source_scalar_type is real 
                and m_exponent is integer */
            void ExponentNumericPromotion(ScalarType i_source_scalar_type)
            {
                auto const old_scalar_type = GetExponentType();

                ScalarType const new_scalar_type = DeduceScalarType({old_scalar_type, i_source_scalar_type});
                if(new_scalar_type != old_scalar_type)
                {
                    // we expect only Integer to Real promotion
                    if(new_scalar_type != ScalarType::Real || old_scalar_type != ScalarType::Integer)
                        Panic("ExponentNumericPromotion - unexpected promotion from ",
                            old_scalar_type, " to ", new_scalar_type);
  
                      const SharedArray<Integer> & integers = std::get<SharedArray<Integer>>(m_exponent);
                    SharedArray<Real> reals(integers.size());
                    for(size_t i = 0; i < integers.size(); i++)
                        reals[i] = NumericCast<Real>(integers[i]);

                    m_exponent = std::move(reals);
                }
            }

            template <typename EXPONENT_TYPE, typename SOURCE_TYPE>
                bool TryMergeExponentScalars(const TensorValue & i_source)
            {
                auto & exponent = std::get<SharedArray<EXPONENT_TYPE>>(m_exponent);
                for (Indices indices(m_exponent_shape); indices; indices++)
                {
                    auto & dest = exponent[indices.GetLogicalLinearIndex()];
                    auto const source = indices.At<SOURCE_TYPE>(i_source);

                    bool const dest_positive = dest > EXPONENT_TYPE{};
                    bool const source_positive = source > SOURCE_TYPE{};

                    if(dest_positive != source_positive)
                        return false;

                    if(dest_positive)
                    {
                        if(dest < source)
                            dest = NumericCast<EXPONENT_TYPE>(source);
                    }
                    else
                    {
                        if(dest > source)
                            dest = NumericCast<EXPONENT_TYPE>(source);
                    }
                }

                return true;
            }

            bool TryMergeExponent(const TensorValue & i_source_exponent)
            {
                ScalarType const source_scalar_type = i_source_exponent.GetScalarType();

                m_exponent_shape = Broadcast({m_exponent_shape, i_source_exponent.GetShape()});

                ExponentNumericPromotion(source_scalar_type);

                ScalarType const dest_scalar_type = GetExponentType();
                if(dest_scalar_type == ScalarType::Integer)
                {
                    if(source_scalar_type == ScalarType::Integer)
                        return TryMergeExponentScalars<Integer, Integer>(i_source_exponent);
                    else
                        Panic("MergeExponent - internal error - source was expected to be integral");
                }
                else if(dest_scalar_type == ScalarType::Real)
                {
                    if(source_scalar_type == ScalarType::Integer)
                        return TryMergeExponentScalars<Real, Integer>(i_source_exponent);
                    if(source_scalar_type == ScalarType::Real)
                        return TryMergeExponentScalars<Real, Real>(i_source_exponent);
                    else
                        Panic("MergeExponent - internal error - source was expected to be real or integral");
                }
                else
                    Panic("MergeExponent - internal error - destination was expected to be real or integral");
            }
        };

    } // detail
    
    std::optional<Tensor> Factorize(Span<const Tensor> i_terms)
    {
        std::vector<Factor> factors;

        for(size_t term_index = 0; term_index < i_terms.size(); term_index++)
        {
            EnumFactors(i_terms[term_index], [&](const Tensor & i_base, const TensorValue & i_exponent) {

                bool found = false;
                for(auto & factor : factors)
                {
                    if(AreIdentical(factor.m_base, i_base) && factor.TryMergeExponent(i_exponent))
                    {
                        factor.m_term_indices.push_back(term_index);
                        found = true;
                        break;
                    }
                }
                if(!found)
                    factors.emplace_back(i_base, i_exponent, term_index);
            });
        }

        if(!factors.empty())
        {
            auto const most_requent = std::max_element(factors.begin(), factors.end(), 
                [](const Factor & i_first, const Factor & i_second){
                    return i_first.m_term_indices.size() < i_second.m_term_indices.size(); });

            if(most_requent->m_term_indices.size() >= 2)
            {
                Tensor const factor = Pow(most_requent->m_base, 
                    MakeConstant(most_requent->GetAndResetExponent()));
                std::vector<Tensor> factorized_terms, non_factorized_terms;
                for(size_t term_index = 0; term_index < i_terms.size(); term_index++)
                {
                    if(Contains(most_requent->m_term_indices, term_index))
                        factorized_terms.push_back(i_terms[term_index] / factor);
                    else
                        non_factorized_terms.push_back(i_terms[term_index]);
                }

                if(non_factorized_terms.empty())
                    return Add(factorized_terms) * factor;
                else
                    return Add(non_factorized_terms) + Add(factorized_terms) * factor;
            }
        }

        return {};
    }
}
