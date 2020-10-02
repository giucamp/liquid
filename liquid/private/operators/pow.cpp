
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"
#include "tensor_type.h"
#include "indices.h"
#include <cmath>

namespace liquid
{
    TensorValue PowEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        const FixedShape & result_shape = i_result_type.GetFixedShape();
        SharedArray<Real> result(static_cast<size_t>(result_shape.GetLinearSize()));

        const TensorValue & base = i_operands.at(0);
        const TensorValue & exponent = i_operands.at(1);

        if(exponent.GetScalarType() == ScalarType::Real)
        {
            for (Indices indices(result_shape); indices; indices++)
            {
                auto const base_el = indices.At<Real>(base);
                auto const exponent_el = indices.At<Real>(exponent);
                indices[result] = std::pow(base_el, exponent_el);
            }
        }
        else if(exponent.GetScalarType() == ScalarType::Integer)
        {
            for (Indices indices(result_shape); indices; indices++)
            {
                auto const base_el = indices.At<Real>(base);
                auto const exponent_el = indices.At<Integer>(exponent);
                indices[result] = std::pow(base_el, exponent_el);
            }
        }
        else
            Panic("Pow - unexpected exponent type: ", exponent.GetScalarType());
        return TensorValue(std::move(result), result_shape);
    }

    Tensor PowGradient(const Tensor & i_self,
        const Tensor & i_self_gradient, size_t i_operand_index)
    {
        const Tensor & base = i_self.GetExpression()->GetOperands().at(0);
        const Tensor & exponent = i_self.GetExpression()->GetOperands().at(1);
        if(i_operand_index == 0)
            return i_self_gradient * i_self * exponent / base;
            // return i_self_gradient * exponent * Pow(base, exponent - 1);
        else
            return i_self_gradient * i_self * Log(base);
    }

    extern const Operator & GetOperatorPow()
    {
        static auto const op = Operator("Pow")
            .AddOverload(PowEvaluate, { {GetScalarType<Real>(), "Base"}, {GetScalarType<Integer>(), "Exponent"} } )
            .AddOverload(PowEvaluate, { {GetScalarType<Real>(), "Base"}, {GetScalarType<Real>(), "Exponent"} } )
            .SetGradientOfOperand(PowGradient);
        return op;
    }

    Tensor Pow(const Tensor & i_base, const Tensor & i_exponent)
    {
        return GetOperatorPow().Invoke({i_base, i_exponent});
    }

    Tensor Square(const Tensor & i_base)
    {
        return Pow(i_base, 2);
    }
}
