
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
    TensorType IfDeduceType(const std::any& i_attachment, Span<const Tensor> i_operands)
    {
        size_t const condition_count = i_operands.size() / 2;

        // sintax: If([Condition, Value]*, Fallback)
        
        // only value operands partecipate in scalar type deduction
        std::vector<ScalarType> scalar_types;
        scalar_types.reserve(condition_count + 1);
        for (size_t condition_index = 0; condition_index < condition_count; condition_index++)
        {
            const TensorType & value_type = i_operands.at(condition_index * 2 + 1).GetExpression()->GetType();
            scalar_types.push_back(value_type.GetScalarType());
        }
        const TensorType & fallback_value_type = i_operands.back().GetExpression()->GetType();
        scalar_types.push_back(fallback_value_type.GetScalarType());

        // all operands partecipate in shape deduction
        std::vector<FixedShape> shapes;
        shapes.reserve(i_operands.size());
        for (size_t operand_index = 0; operand_index < i_operands.size(); operand_index++)
        {
            const TensorType & operand_type = i_operands[operand_index].GetExpression()->GetType();
            if(operand_type.HasFixedShape())
                shapes.push_back(operand_type.GetFixedShape());
        }

        ScalarType const scalar_type = DeduceScalarType(scalar_types);
        if(shapes.size() == i_operands.size())
            return { scalar_type, Broadcast(shapes) };
        else
            return { scalar_type };
    }

    template <typename SCALAR_TYPE>
        TensorValue IfEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        size_t const condition_count = i_operands.size() / 2;

        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<SCALAR_TYPE> result(static_cast<size_t>(result_shape.GetLinearSize()));

        /* 'If' is evaluated component-wise: for every element of the conditions we select the
            corresponding element of a value. */
        for (Indices indices(result_shape); indices; indices++)
        {
            bool assigned = false;
            for (size_t condition_index = 0; condition_index < condition_count && !assigned; condition_index++)
            {
                Bool const condition = indices.At<Bool>(i_operands[condition_index * 2]);
                if(condition)
                {
                    const TensorValue & value = i_operands[condition_index * 2 + 1];
                    indices[result] = indices.At<SCALAR_TYPE>(value);
                    assigned = true;
                }
            }
            if(!assigned)
            {
                const TensorValue & fallback_value = i_operands.back();
                indices[result] = indices.At<SCALAR_TYPE>(fallback_value);
            }
        }

        return TensorValue(std::move(result), result_shape);
    }

    Tensor IfGradient([[maybe_unused]] const Tensor & i_self,
        const Tensor & i_self_gradient, [[maybe_unused]] size_t i_operand_index)
    {
        const std::vector<Tensor> & source_ops = i_self.GetExpression()->GetOperands();

        size_t const operand_count = source_ops.size();
        if((i_operand_index % 2) == 0 && i_operand_index + 1 != operand_count)
            Panic("Trying to compute the gradient of a condition");
        
        size_t const condition_count = source_ops.size() / 2;

        std::vector<Tensor> grad_ops = source_ops;
        for (size_t condition_index = 0; condition_index < condition_count; condition_index++)
        {
            Tensor & value = grad_ops[condition_index * 2 + 1];
            if (i_operand_index == condition_index * 2 + 1)
                value = i_self_gradient;
            else
                value = Real{};
        }
        
        if(i_operand_index + 1 == grad_ops.size())
            grad_ops.back() = i_self_gradient;
        else
            grad_ops.back() = Real{};

        return If(grad_ops);
    }

    const char g_if_description[] = 
        "Performs a component-wise value selection based on a set a conditions.\n"
        "The return value is a tensor in which every element is taken from the first"
        " 'Value' operand that has its corresponding element in the preceding 'Condition'" 
        " equal to true, or from the 'Fallback' operand if there is no corresponding"
        " condition element equal to true.";

    const char g_if_return_type[] = 
        "The return scalar type is deduced permorming numeric promotion from all"
        " 'Value' operands and the 'Fallback' operand.\n"
        "The return shape is deduced by broadcasting the shapes of all operands, "
        " including conditions.";

    extern const Operator & GetOperatorIf()
    {
        static auto const op = Operator("If")
            .SetDoc(g_if_description, g_if_return_type)
            .SetDeduceType(IfDeduceType)
            .AddOverload(IfEvaluate<Real>, {
                { GetScalarType<Bool>(), "Condition" },
                { GetScalarType<Real>(), "Value" },
                { GetScalarType<Real>(), "Fallback" }  }, 2) // the first 2 parameters are the variadic pack
            .AddOverload(IfEvaluate<Integer>, {
                { GetScalarType<Bool>(), "Condition" },
                { GetScalarType<Integer>(), "Value" },
                { GetScalarType<Integer>(), "Fallback" }  }, 2) // the first 2 parameters are the variadic pack
            .AddOverload(IfEvaluate<Bool>, {
                { GetScalarType<Bool>(), "Condition" },
                { GetScalarType<Bool>(), "Value" },
                { GetScalarType<Bool>(), "Fallback" }  }, 2) // the first 2 parameters are the variadic pack
            .SetGradientOfOperand(IfGradient);
        return op;
    }

    Tensor If(Span<Tensor const> i_operands)
    {
        return GetOperatorIf().Invoke(i_operands);
    }
}
