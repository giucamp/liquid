
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"

namespace liquid
{
    TensorType ConstantDeduceType(const std::any& i_attachment, 
        [[maybe_unused]] Span<const Tensor> i_operands, 
        [[maybe_unused]] Span<const Tensor> i_attributes)
    {
        return std::any_cast<const TensorValue &>(i_attachment).GetType();
    }

    TensorValue ConstantEvaluate(const std::any & i_attachment,
        [[maybe_unused]] const TensorType & i_result_type,
        [[maybe_unused]] Span<const TensorValue> i_operands,
        [[maybe_unused]] Span<const TensorValue> i_attributes)
    {
        return std::any_cast<TensorValue>(i_attachment);
    }

    extern const Operator & GetOperatorConstant()
    {
        static auto const op = Operator("Constant")
            .SetDeduceType(ConstantDeduceType)
            .AddOverload({ ConstantEvaluate, { }, { } });
        return op;
    }

    Tensor MakeConstant(const TensorValue & i_value)
    {
        return GetOperatorConstant().Invoke({}, {}, i_value);
    }

    bool IsConstant(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression()->OperatorIs(GetOperatorConstant());
    }

    const TensorValue & GetConstantValue(const Tensor & i_tensor)
    {
        if(!IsConstant(i_tensor))
            Panic("GetConstantValue - not a constant tensor");
        return std::any_cast<const TensorValue&>(i_tensor.GetExpression()->GetAttachment());
    }

    template Span<const Real> GetConstantStorage(const Tensor & i_tensor);
    template Span<const Integer> GetConstantStorage(const Tensor & i_tensor);
    template Span<const Bool> GetConstantStorage(const Tensor & i_tensor);

    template <typename SCALAR_TYPE>
        Span<const SCALAR_TYPE> GetConstantStorage(const Tensor & i_tensor)
    {
        if(i_tensor.GetScalarType() != GetScalarType<SCALAR_TYPE>())
            Panic("GetConstantStorage - mismatching type, tensor is ", i_tensor.GetScalarType(),
                ", requested type ", GetScalarType<SCALAR_TYPE>());
        return GetConstantValue(i_tensor).GetAs<SCALAR_TYPE>();
    }

    template std::vector<Real> ConstantToVector(const Tensor & i_tensor);
    template std::vector<Integer> ConstantToVector(const Tensor & i_tensor);
    template std::vector<Bool> ConstantToVector(const Tensor & i_tensor);

    template <typename SCALAR_TYPE>
        std::vector<SCALAR_TYPE> ConstantToVector(const Tensor & i_tensor)
    {
        auto const storage = GetConstantStorage<SCALAR_TYPE>(i_tensor);
        auto const & shape = i_tensor.GetExpression()->GetType().GetFixedShape();
        auto const linear_size = static_cast<size_t>(shape.GetLinearSize());
        
        std::vector<SCALAR_TYPE> result;
        result.reserve(linear_size);

        // unwrap the constant
        while(result.size() < linear_size)
            result.insert(result.end(), storage.begin(), storage.end());

        return result;
    }
}
