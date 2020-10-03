
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
    TensorType StackDeduceType([[maybe_unused]] const std::any & i_attachment,
        Span<const Tensor> i_operands,
        [[maybe_unused]] Span<const Tensor> i_attributes)
    {
        auto const common_type = DeduceType(i_operands);

        if(common_type.HasFixedShape())
        {
            Span<const Integer> const source_dimensions = common_type.GetFixedShape().GetDimensions();

            Integer axis = 0;

            std::vector<Integer> dest_dimenions;
            dest_dimenions.reserve(source_dimensions.size() + 1);
            dest_dimenions.insert(dest_dimenions.begin(), source_dimensions.begin(), source_dimensions.end());
            dest_dimenions.insert(dest_dimenions.begin() + axis, NumericCast<Integer>(i_operands.size()) );

            return { common_type.GetScalarType(), FixedShape(dest_dimenions) };
        }
        else
            return common_type.GetScalarType();
    }

    template <typename SCALAR_TYPE>
        TensorValue StackEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<SCALAR_TYPE> result(static_cast<size_t>(result_shape.GetLinearSize()));

        Integer axis = 0;

        std::vector<Integer> result_indices;
        for (size_t operand_index = 0; operand_index < i_operands.size(); operand_index++)
        {
            const FixedShape & op_shape = i_operands[operand_index].GetShape();
            for (Indices source_indices(op_shape); source_indices; source_indices++)
            {
                result_indices = source_indices.GetIndices();
                result_indices.insert(result_indices.begin() + axis, NumericCast<Integer>(operand_index));

                Integer const linear_index = result_shape.GetPhysicalLinearIndex(result_indices);
                result[linear_index] = source_indices.At<SCALAR_TYPE>(i_operands[operand_index]);
            }
        }

        return TensorValue(std::move(result), result_shape);
    }

    extern const Operator & GetOperatorStack()
    {
        static auto const op = Operator("Stack")
            .AddOverload(StackEvaluate<Real>, { {GetScalarType<Real>(), "Source"} }, 1 )
            .AddOverload(StackEvaluate<Integer>, { {GetScalarType<Integer>(), "Source"} }, 1 )
            .AddOverload(StackEvaluate<Bool>, { {GetScalarType<Bool>(), "Source"} }, 1 );
        return op;
    }

    Tensor Stack(Span<Tensor const> i_tensors)
    {
        return GetOperatorStack().Invoke(i_tensors);
    }
}
