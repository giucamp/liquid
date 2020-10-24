
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
    extern const Operator & GetOperatorPow();

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

    std::optional<Tensor> PowCanonicalizeReplace(const Tensor & i_source)
    {
        const Tensor & base = i_source.GetExpression()->GetOperands().at(0);
        const Tensor & exponent = i_source.GetExpression()->GetOperands().at(1);
        const Tensor & zero = MakeConstant<0>();
        const Tensor & one = MakeConstant<1>();

        /* pow(real a, 0) where a != 0 -> 1
           pow(0, 0) -> undeterminate (see https://en.wikipedia.org/wiki/Zero_to_the_power_of_zero) */
        if(AreIdentical(exponent, zero))
        {
            if(AreIdentical(base, zero))
                return {}; // do nothing
            else
                return one;
        }

        // pow(real a, 1) -> a
        if(AreIdentical(exponent, one))
            return base;

        // pow(0, real a) where a != 0 -> 0
        if(AreIdentical(base, zero))
            return zero;

        // pow(1, real a) -> 1
        if(AreIdentical(base, one))
            return one;

        // (real a ^ real b) ^ real c -> a ^ (b*c)
        // or: pow(pow(real a, real b), real c) -> pow(a, b*c)
        if(base.GetExpression()->OperatorIs(GetOperatorPow()))
        {
            const Tensor & other_base = base.GetExpression()->GetOperand(0); // a
            const Tensor & other_exponent = base.GetExpression()->GetOperand(1); // b
            return Pow(other_base, exponent * other_exponent);
        }

        return {};
    }

    extern const Operator & GetOperatorPow()
    {
        static auto const op = Operator("pow")
            .AddCanonicalize(PowCanonicalizeReplace)
            .AddOverload(PowEvaluate, { {ScalarType::Real, "base"}, {ScalarType::Integer, "exponent"} } )
            .AddOverload(PowEvaluate, { {ScalarType::Real, "base"}, {ScalarType::Real, "exponent"} } )
            .SetGradientOfOperand(PowGradient);
        return op;
    }

    Tensor Pow(const Tensor & i_base, const Tensor & i_exponent)
    {
        return GetOperatorPow().Invoke({i_base, i_exponent});
    }

    Tensor Square(const Tensor & i_source)
    {
        return Pow(i_source, 2);
    }
}
