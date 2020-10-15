
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
    TensorType EqualDeduceType( [[maybe_unused]] const std::any & i_attachment,
        [[maybe_unused]] Span<const Tensor> i_operands)
    {
        const TensorType & first_type = i_operands.at(0).GetExpression()->GetType();
        const TensorType & second_type = i_operands.at(1).GetExpression()->GetType();

        if(first_type.HasFixedShape() && second_type.HasFixedShape())
            return { ScalarType::Bool, Broadcast({first_type.GetFixedShape(), second_type.GetFixedShape()}) };
        else
            return { ScalarType::Bool };
    }

    template <typename SCALAR_TYPE>
        TensorValue EqualEvaluate(const TensorType& i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape& result_shape = i_result_type.GetFixedShape();
        SharedArray<Bool> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            const SCALAR_TYPE & first = indices.At<SCALAR_TYPE>(i_operands[0]);
            const SCALAR_TYPE & second = indices.At<SCALAR_TYPE>(i_operands[1]);
            indices[result] = first == second;
        }

        return TensorValue(std::move(result), result_shape);
    }

    extern const Operator & GetOperatorEqual()
    {
        static auto const op = Operator("Equal")
            .AddFlags(Operator::Flags::Commutative)
            .SetDeduceType(EqualDeduceType)
            .AddOverload(EqualEvaluate<Real>, { { ScalarType::Real, "First" }, { ScalarType::Real, "Second" } })
            .AddOverload(EqualEvaluate<Integer>, { { ScalarType::Integer, "First" }, { ScalarType::Integer, "Second" } })
            .AddOverload(EqualEvaluate<Bool>, { { ScalarType::Bool, "First" }, { ScalarType::Bool, "Second" } });
        return op;
    }

    Tensor Equal(const Tensor & i_first, const Tensor & i_second)
    {
        return GetOperatorEqual().Invoke({ i_first, i_second });
    }

    Tensor operator == (const Tensor & i_first, const Tensor & i_second)
    {
        return Equal(i_first, i_second);
    }

    Tensor operator != (const Tensor & i_first, const Tensor & i_second)
    {
        return !Equal(i_first, i_second);
    }
}

