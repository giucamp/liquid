
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
    TensorValue AndEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<Bool> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            Bool element = true;
            for(const TensorValue & operand : i_operands)
                element = element && indices.At<Bool>(operand);
            indices[result] = element;
        }

        return TensorValue(std::move(result), result_shape);
    }

    extern const Operator & GetOperatorAnd()
    {
        static auto const op = Operator("And")
            .AddFlags(Operator::Flags::Commutative | Operator::Flags::Associative)
            .SetIdentityElement(true)
            .AddOverload(AndEvaluate, { { ScalarType::Bool, "BoolOp" } }, 1);
        return op;
    }

    Tensor And(Span<Tensor const> i_bool_operands)
    {
        return GetOperatorAnd().Invoke(i_bool_operands);
    }

    Tensor operator && (const Tensor & i_first_bool, const Tensor & i_second_bool)
    {
        return And({ i_first_bool, i_second_bool });
    }
}
