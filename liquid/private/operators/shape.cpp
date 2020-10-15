
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
    TensorType ShapeDeduceType(const std::any & i_attachment,
        Span<const Tensor> i_operands)
    {
        // shape_of_shape is a vector
        Tensor const shape_of_shape = { { Rank(i_operands.at(0)) } };
        return { ScalarType::Integer, shape_of_shape };
    }

    bool ShapeEligibleForPropagation(const std::any & i_attachment,
        Span<const Tensor> i_operands)
    {
        const TensorType & type = i_operands.at(0).GetExpression()->GetType();
        return type.HasFixedShape();
    }

    TensorValue ShapeEvaluate([[maybe_unused]] const std::any & i_attachment,
        const TensorType & i_result_type, Span<const Tensor> i_operands)
    {
        const FixedShape & source_shape = i_operands.at(0).GetExpression()->GetType().GetFixedShape();
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        Span<const Integer> const source_dimensions = source_shape.GetDimensions();

        SharedArray<Integer> result(static_cast<size_t>(result_shape.GetLinearSize()));
        for (size_t i = 0; i < result.size(); i++)
            result[i] = source_dimensions[i];

        return TensorValue(std::move(result), i_result_type.GetFixedShape());
    }

    extern const Operator & GetOperatorShape()
    {
        static auto const op = Operator("Shape")
            .SetDeduceType(ShapeDeduceType)
            .SetEligibleForPropagation(ShapeEligibleForPropagation)
            .AddOverload(ShapeEvaluate, {{ GetScalarType<Real>(), "Source" }} )
            .AddOverload(ShapeEvaluate, {{ GetScalarType<Integer>(), "Source" }} )
            .AddOverload(ShapeEvaluate, {{ GetScalarType<Bool>(), "Source" }} );
        return op;
    }

    Tensor Shape(const Tensor & i_tensor)
    {
        return GetOperatorShape().Invoke({ i_tensor });
    }
}
