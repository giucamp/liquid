
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

    Tensor MulGradient([[maybe_unused]] const Tensor& i_self,
        const Tensor& i_self_gradient, [[maybe_unused]] size_t i_operand_index)
    {
        std::vector<Tensor> operands = i_self.GetExpression()->GetOperands();
        operands[i_operand_index] = i_self_gradient;
        return Mul(operands);
    }

    extern const Operator & GetOperatorMul()
    {
        static auto const op = Operator("Mul")
            .AddFlags(Operator::Flags::Commutative | Operator::Flags::Associative)
            .SetIdentityElement(1)
            .AddOverload(MulEvaluate<Real>, { {GetScalarType<Real>(), "Factor"} }, 1)
            .AddOverload(MulEvaluate<Integer>, { {GetScalarType<Integer>(), "Factor"} }, 1)
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

    Tensor & operator *= (Tensor & i_first, const Tensor & i_second)
    {
        i_first = i_first * i_second;
        return i_first;
    }
}
