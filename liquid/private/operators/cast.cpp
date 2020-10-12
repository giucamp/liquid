
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
    std::optional<Tensor> CastCanonicalize(const Tensor & i_cast)
    {
        auto const & source = i_cast.GetExpression()->GetOperands().at(0);
        auto const source_type = source.GetExpression()->GetType().GetScalarType();
        auto const dest_type = i_cast.GetExpression()->GetType().GetScalarType();
        
        if(dest_type == source_type)
            return source;
        else
            return {};
    }

    TensorType CastDeduceType(const std::any & i_attachment, Span<const Tensor> i_operands)
    {
        auto const dest_scalar_type = std::any_cast<ScalarType>(i_attachment);
        auto const & source_shape = i_operands.at(0).GetExpression()->GetType().GetShape();
        return { dest_scalar_type, source_shape };
    }

    template <typename DEST_TYPE, typename SOURCE_TYPE>
        TensorValue CastEvaluateImpl(const TensorValue & i_source)
    {
        const FixedShape & result_shape = i_source.GetShape();

        SharedArray<DEST_TYPE> result(static_cast<size_t>(result_shape.GetLinearSize()));
        for (Indices indices(result_shape); indices; indices++)
            indices[result] = static_cast<DEST_TYPE>(indices.At<SOURCE_TYPE>(i_source));

        return TensorValue(std::move(result), result_shape);
    }

    template <typename SOURCE_TYPE>
        TensorValue CastEvaluate(const std::any & i_attachment,
            const TensorType & i_result_type,
            Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();        
        auto const dest_type = std::any_cast<ScalarType>(i_attachment);
        if(i_result_type.GetScalarType() != dest_type)
            Panic("CastEvaluate - internal error - mismatching types");

        switch (dest_type)
        {
        case ScalarType::Real:
            return CastEvaluateImpl<Real, SOURCE_TYPE>(i_operands.at(0));

        case ScalarType::Integer:
            return CastEvaluateImpl<Integer, SOURCE_TYPE>(i_operands.at(0));

        default:
            Panic("CastEvaluate - unrecognized dest type");
        }  
    }

    extern const Operator & GetOperatorCast()
    {
        static auto const op = Operator("Add")
            .SetDeduceType(CastDeduceType)
            .AddOverload(CastEvaluate<Real>, { { GetScalarType<Real>(), "Source" } })
            .AddOverload(CastEvaluate<Integer>, { { GetScalarType<Integer>(), "Source" } })
            .SetAttachmentComparer<ScalarType>()
            .SetAttachmentHasher<ScalarType>()
            .SetCanonicalize(CastCanonicalize);
        return op;
    }

    Tensor Cast(ScalarType i_dest_scalar_type, const Tensor & i_source)
    {
        return GetOperatorCast().Invoke({i_source}, i_dest_scalar_type);
    }
}
