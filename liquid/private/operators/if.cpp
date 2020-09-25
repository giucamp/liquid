
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
        TensorValue IfEvaluate(const TensorType& i_result_type, Span<const TensorValue> i_operands)
    {
        const Shape& result_shape = i_result_type.GetFixedShape();
        SharedArray<SCALAR_TYPE> result(static_cast<size_t>(result_shape.GetLinearSize()));

        /*for (Indices indices(result_shape); indices; indices++)
        {
            SCALAR_TYPE element = {};
            for (const TensorValue& operand : i_operands)
                element += indices.At<SCALAR_TYPE>(operand);
            indices[result] = element;
        }*/

        return TensorValue(std::move(result), result_shape);
    }

    Tensor IfGradient([[maybe_unused]] const Tensor& i_self,
        const Tensor& i_self_gradient, [[maybe_unused]] size_t i_operand_index)
    {
        return i_self_gradient;
    }

    extern const Operator& GetOperatorIf()
    {
        static auto const op = Operator("If")
            .AddOverload({ IfEvaluate<Real>, { GetScalarType<Real>() }, { "Condition", "Result", "Fallback" }, 2 })
            .AddOverload({ IfEvaluate<Integer>, { GetScalarType<Integer>() }, { "Condition", "Result", "Fallback" }, 2 })
            .AddOverload({ IfEvaluate<Bool>, { GetScalarType<Bool>() }, { "Condition", "Result", "Fallback" }, 2 })
            .SetGradientOfOperand(IfGradient);
        return op;
    }
}
