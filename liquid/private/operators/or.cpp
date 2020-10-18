
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
    TensorValue OrEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<Bool> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            Bool element = false;
            for(const TensorValue & operand : i_operands)
                element = element || indices.At<Bool>(operand);
            indices[result] = element;
        }

        return TensorValue(std::move(result), result_shape);
    }

    extern const Operator & GetOperatorOr()
    {
        static auto const op = Operator("or")
            .AddFlags(Operator::Flags::Commutative | Operator::Flags::Associative)
            .AddOverload({ OrEvaluate, { { ScalarType::Bool, "bool_expr" } }, 1 });
        return op;
    }

    Tensor Or(Span<Tensor const> i_bool_operands)
    {
        return GetOperatorOr().Invoke(i_bool_operands);
    }

    Tensor operator || (const Tensor & i_first_bool, const Tensor & i_second_bool)
    {
        return Or({ i_first_bool, i_second_bool });
    }
}
