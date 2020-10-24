
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"
#include "tensor_type.h"
#include "indices.h"

namespace liquid
{
    extern const Operator & GetOperatorPow();

    template <typename SCALAR_TYPE>
        TensorValue MulEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<SCALAR_TYPE> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            auto element = static_cast<SCALAR_TYPE>(1);
            for(const TensorValue & operand : i_operands)
                element *= indices.At<SCALAR_TYPE>(operand);
            indices[result] = element;
        }

        return TensorValue(std::move(result), result_shape);
    }

    Tensor MulGradient(const Tensor & i_self,
        const Tensor& i_self_gradient, size_t i_operand_index)
    {
        std::vector<Tensor> operands = i_self.GetExpression()->GetOperands();
        operands[i_operand_index] = i_self_gradient;
        return Mul(operands);
    }

    const char g_mul_description[] = 
        "Returns the component-wise product of the operands.";

    const char g_mul_return_type[] = 
        "The return scalar type is deduced permorming numeric promotion from all operands.\n"
        "The return shape is deduced by broadcasting the shapes of all operands";

    std::optional<Tensor> MulCanonicalizeReplace(const Tensor & i_source)
    {
        const Tensor & zero = MakeConstant<0>();
        for(const auto & operand : i_source.GetExpression()->GetOperands())
            if(AreIdentical(operand, zero))
                return zero;

        return {};
    }

    bool MulCanonicalizeAdjust(std::vector<Tensor> & i_operands)
    {
        bool some_adjustement = false;

        /* pow(a, b) * ... * pow(a, c) -> pow(a, b+c) * ...
            (merge pow's with the same base) */
        for(size_t i = 0; i < i_operands.size(); i++)
        {
            const auto & operand = i_operands[i];
            if(!operand.GetExpression()->OperatorIs(GetOperatorPow()))
                continue;

            const Tensor & base = operand.GetExpression()->GetOperand(0);
            Tensor new_exponent = operand.GetExpression()->GetOperand(1);
            size_t matches = 0; // sumber of subsequent pow's with identical base

            /* serach other pow's with a matching base. On a match,
                the exponent is summed to new_exponent, and the operand
                is removed. */
            for(size_t j = i + 1; j < i_operands.size(); )
            { 
                const auto & other_operand = i_operands[j];
                if(other_operand.GetExpression()->OperatorIs(GetOperatorPow()))
                {
                    const Tensor & other_base = other_operand.GetExpression()->GetOperand(0);
                    const Tensor & other_exponent = other_operand.GetExpression()->GetOperand(1);
                    if(AreIdentical(base, other_base))
                    {
                        // accumulate exponents
                        new_exponent += other_exponent;

                        i_operands.erase(i_operands.begin() + j);
                        matches++;
                        continue;
                    }
                }
                j++;
            }

            // replace the i-th operand with the merged pow
            if(matches > 0)
            {
                i_operands[i] = Pow(base, new_exponent);
                some_adjustement = true;
            }
        }

        return some_adjustement;
    }


    extern const Operator & GetOperatorMul()
    {
        static auto const op = Operator("mul")
            .SetDoc(g_mul_description, g_mul_return_type)
            .AddFlags(Operator::Flags::Commutative | Operator::Flags::Associative)
            .AddCanonicalize(MulCanonicalizeAdjust)
            .AddCanonicalize(MulCanonicalizeReplace)
            .AddOverload(MulEvaluate<Real>, { {ScalarType::Real, "factor"} }, 1)
            .AddOverload(MulEvaluate<Integer>, { {ScalarType::Integer, "factor"} }, 1)
            .SetGradientOfOperand(MulGradient);
        return op;
    }

    Tensor Mul(Span<Tensor const> i_factors)
    {
        return GetOperatorMul().Invoke(i_factors);
    }

    Tensor operator * (const Tensor & i_first, const Tensor& i_second)
    {
        return Mul({ i_first, i_second });
    }

    Tensor operator / (const Tensor & i_dividend, const Tensor & i_divisor)
    {
        return i_dividend * Pow(i_divisor, MakeConstant<-1>());
    }

    Tensor & operator *= (Tensor & i_first, const Tensor & i_second)
    {
        return i_first = i_first * i_second;
    }

    Tensor & operator /= (Tensor & i_dividend, const Tensor & i_divisor)
    {
        return i_dividend = i_dividend / i_divisor;
    }
}
