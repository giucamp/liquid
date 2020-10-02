
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
    template <typename SCALAR_TYPE>
        TensorValue AddEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<SCALAR_TYPE> result(static_cast<size_t>(result_shape.GetLinearSize()));

        for (Indices indices(result_shape); indices; indices++)
        {
            SCALAR_TYPE element = {};
            for(const TensorValue & operand : i_operands)
                element += indices.At<SCALAR_TYPE>(operand);
            indices[result] = element;
        }

        return TensorValue(std::move(result), result_shape);
    }

    Tensor AddGradient([[maybe_unused]] const Tensor & i_self,
        const Tensor & i_self_gradient, [[maybe_unused]] size_t i_operand_index)
    {
        return i_self_gradient;
    }

    extern const Operator & GetOperatorAdd()
    {
        static auto const op = Operator("Add")
            .AddFlags(Operator::Flags::Commutative | Operator::Flags::Associative)
            .SetIdentityElement(0)
            .AddOverload(AddEvaluate<Real>, { {GetScalarType<Real>(), "Addend"} }, 1)
            .AddOverload(AddEvaluate<Integer>, { {GetScalarType<Integer>(), "Addend"} }, 1)
            .SetGradientOfOperand(AddGradient);
        return op;
    }

    Tensor Add(Span<Tensor const> i_addends)
    {
        return GetOperatorAdd().Invoke(i_addends);
    }

    Tensor operator + (const Tensor & i_operand)
    {
        return i_operand;
    }

    Tensor operator - (const Tensor & i_operand)
    {
        return i_operand * -1;
    }

    Tensor operator + (const Tensor & i_first, const Tensor & i_second)
    {
        return Add({ i_first, i_second });
    }

    Tensor operator - (const Tensor & i_first, const Tensor & i_second)
    {
        return Add({ i_first, -i_second });
    }

    Tensor & operator += (Tensor & i_first, const Tensor & i_second)
    {
        return i_first = i_first + i_second;
    }

    Tensor & operator -= (Tensor & i_first, const Tensor & i_second)
    {
        return i_first = i_first - i_second;
    }
}
