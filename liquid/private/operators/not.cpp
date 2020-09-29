
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
    TensorValue NotEvaluate(const TensorType & i_result_type,
        const TensorValue & i_operand)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<Bool> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
            indices[result] = !indices.At<Bool>(i_operand);

        return TensorValue(std::move(result), result_shape);
    }

    extern const Operator & GetOperatorNot()
    {
        static auto const op = Operator("Not")
            .AddOverload({ NotEvaluate, { GetScalarType<Bool>() }, { "BoolOp" } });
        return op;
    }

    Tensor Not(Tensor const & i_bool_operand)
    {
        return GetOperatorNot().Invoke(i_bool_operand);
    }

    Tensor operator ! (const Tensor & i_bool_operand)
    {
        return Not(i_bool_operand);
    }
}
