
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
    TensorType RankDeduceType( [[maybe_unused]] const std::any & i_attachment,
        [[maybe_unused]] Span<const Tensor> i_operands)
    {
        return { ScalarType::Integer, FixedShape::Scalar() };
    }

    bool RankEligibleForPropagation(const std::any & i_attachment,
        Span<const Tensor> i_operands)
    {
        const TensorType & type = i_operands.at(0).GetExpression()->GetType();
        return type.HasFixedShape();
    }

    TensorValue RankEvaluate([[maybe_unused]] const std::any & i_attachment,
        const TensorType & i_result_type, Span<const Tensor> i_operands)
    {
        const Tensor & operand = i_operands.at(0);
        const FixedShape & shape = operand.GetExpression()->GetType().GetFixedShape();
        return TensorValue(SharedArray<Integer>{ shape.GetRank() }, i_result_type.GetFixedShape());
    }

    extern const Operator & GetOperatorRank()
    {
        static auto const op = Operator("Rank")
            .SetDeduceType(RankDeduceType)
            .SetEligibleForPropagation(RankEligibleForPropagation)
            .AddOverload({ RankEvaluate, {{ GetScalarType<Real>(), "Source" } }})
            .AddOverload({ RankEvaluate, {{ GetScalarType<Integer>(), "Source" } }})
            .AddOverload({ RankEvaluate, {{ GetScalarType<Bool>(), "Source" } }});
        return op;
    }

    Tensor Rank(const Tensor & i_tensor)
    {
        return GetOperatorRank().Invoke({ i_tensor });
    }
}
