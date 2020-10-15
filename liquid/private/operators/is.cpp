
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
    /*
        any is any      -> true
        int is any      -> true
        bool is any     -> true
        real is any     -> true

        any is real     -> ?
        int is real     -> false
        bool is real    -> false
        real is real    -> true
    */

    TensorType IsDeduceType(const std::any & i_attachment,
        Span<const Tensor> i_operands)
    {
        Tensor const scalar_shape = { 1 };
        return { ScalarType::Bool, scalar_shape };
    }

    bool IsEligibleForPropagation(const std::any & i_attachment,
        Span<const Tensor> i_operands)
    {
        const TensorType & left = i_operands[0].GetExpression()->GetType();
        const TensorType & right = std::any_cast<const TensorType&>(i_attachment);

        return left.GetScalarType() != ScalarType::Any ||
            right.GetScalarType() == ScalarType::Any;
    }

    TensorValue IsEvaluate(const std::any & i_attachment,
        const TensorType & i_result_type, Span<const Tensor> i_operands)
    {
        const TensorType & left = i_operands[0].GetExpression()->GetType();
        const TensorType & right = std::any_cast<const TensorType &>(i_attachment);

        if(right.GetScalarType() == ScalarType::Any)
            return true;

        if(left.GetScalarType() == ScalarType::Any)
            Panic("(", left.GetScalarType(), " is ", right.GetScalarType(), ") is undecidable");

        if(left.GetScalarType() != right.GetScalarType())
            return false;

        struct Visitor
        {
            const TensorType & m_right;
            
            bool operator()(std::monostate)
            {
                return m_right.HasUndefinedShape();
            }

            bool operator()(const FixedShape & i_left_shape)
            {
                return m_right.HasFixedShape()
                    && i_left_shape == m_right.GetFixedShape();
            }

            bool operator()(const Tensor & i_left_shape)
            {
                return m_right.HasVariableShape()
                    && Always(i_left_shape == m_right.GetVariableShape());
            }
        };

        bool const shape_equal = std::visit(Visitor{right}, left.GetShape());
        return shape_equal;
    }

    extern const Operator & GetOperatorIs()
    {
        static auto const op = Operator("Is")
            .SetDeduceType(IsDeduceType)
            .SetEligibleForPropagation(IsEligibleForPropagation)
            .AddOverload(IsEvaluate, {{ ScalarType::Any, "Source" }} );
        return op;
    }

    Tensor Is(const Tensor & i_tensor, const TensorType & i_type)
    {
        return GetOperatorIs().Invoke({ i_tensor }, {i_type});
    }
}
