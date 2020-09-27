
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
        [[maybe_unused]] Span<const Tensor> i_operands,
        [[maybe_unused]] Span<const Tensor> i_attributes)
    {
        return { ScalarType::Integer, FixedShape::Scalar() };
    }

    TensorValue RankEvaluate(const TensorType & i_result_type, const TensorValue & i_source)
    {
        SharedArray<Integer> result = { i_source.GetShape().GetRank() };
        return TensorValue(std::move(result), i_result_type.GetFixedShape());
    }

    extern const Operator & GetOperatorRank()
    {
        static auto const op = Operator("Rank")
            .SetDeduceType(RankDeduceType)
            .AddOverload({ RankEvaluate, { GetScalarType<Real>() }, { "Source" } })
            .AddOverload({ RankEvaluate, { GetScalarType<Integer>() }, { "Source" } })
            .AddOverload({ RankEvaluate, { GetScalarType<Bool>() }, { "Source" } });
        return op;
    }

    Tensor Rank(const Tensor & i_tensor)
    {
        return GetOperatorRank().Invoke({ i_tensor });
    }
}
