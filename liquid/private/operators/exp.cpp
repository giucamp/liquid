
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"
#include "tensor_type.h"
#include "indices.h"
#include <cmath>

namespace liquid
{
    TensorValue ExpEvaluate(const TensorType & i_result_type, const TensorValue & i_operand)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<Real> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            auto const element = indices.At<Real>(i_operand);
            indices[result] = std::exp(element);
        }

        return TensorValue(std::move(result), result_shape);
    }

    Tensor ExpGradient(const Tensor & i_self,
        const Tensor & i_self_gradient, [[maybe_unused]] size_t i_operand_index)
    {
        return i_self_gradient * i_self; 
    }

    extern const Operator & GetOperatorExp()
    {
        static auto const op = Operator("exp")
            .AddOverload(ExpEvaluate, { {ScalarType::Real, "operand"} } )
            .SetGradientOfOperand(ExpGradient);
        return op;
    }

    Tensor Exp(const Tensor & i_operand)
    {
        return GetOperatorExp().Invoke({i_operand});
    }
}
