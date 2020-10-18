
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
    TensorValue SinEvaluate(const TensorType & i_result_type, const TensorValue & i_operand)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<Real> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            auto const element = indices.At<Real>(i_operand);
            indices[result] = std::sin(element);
        }

        return TensorValue(std::move(result), result_shape);
    }

    Tensor SinGradient(const Tensor & i_self,
        const Tensor & i_self_gradient, size_t i_operand_index)
    {
        const Tensor & operand = i_self.GetExpression()->GetOperands().at(0);
        return i_self_gradient * Cos(operand); 
    }

    extern const Operator & GetOperatorSin()
    {
        static auto const op = Operator("sin")
            .AddOverload(SinEvaluate, { {ScalarType::Real, "operand"} } )
            .SetGradientOfOperand(SinGradient);
        return op;
    }

    Tensor Sin(const Tensor & i_operand)
    {
        return GetOperatorSin().Invoke({i_operand});
    }
}
