
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "liquid.h"
#include "tensor_value.h"
#include "tensor_type.h"
#include <optional>
#include <any>
#include <variant>
#include <string_view>

namespace liquid
{
    class Operator
    {
    public:
 
        enum class Flags
        {
            None = 0,
            Commutative = 1 << 0,
            Associative = 1 << 1
        };

        friend Flags operator | (Flags i_first, Flags i_second)
        {
            return static_cast<Flags>(static_cast<int>(i_first) | static_cast<int>(i_second));
        }

        using DeduceTypeFunction = TensorType(*)(const std::any & i_attachment, Span<const Tensor> i_operands, Span<const Tensor> i_attributes);

        using EvaluateFunction = TensorValue(*)(const TensorType& i_result_type, Span<const TensorValue> i_operands);

        using EvaluateWithAttachmentFunction = TensorValue(*)(const std::any & i_attachment, const TensorType & i_result_type, Span<const TensorValue> i_operands);

        using SimplifyFunction = std::optional<Tensor>(*)(const Tensor & );

        using GradientOfOperandFunction = Tensor(*)(const Tensor & i_self, const Tensor & i_self_gradient, size_t i_operand_index);

        struct Overload
        {
            std::variant<EvaluateFunction, EvaluateWithAttachmentFunction> m_evaluate = {};
            std::vector<TensorType> m_parameter_types;
            std::vector<std::string> m_parameter_names;
            size_t m_variadic_parameters_count = {};
        };

        Operator(std::string_view i_name);

        Operator & AddFlags(Flags i_flags);

        Operator & SetDeduceType(DeduceTypeFunction i_func);
        
        Operator & AddOverload(const Overload& i_overload);

        Operator & SetSimplify(const SimplifyFunction & i_func);

        Operator & SetGradientOfOperand(GradientOfOperandFunction i_func);

        Tensor Invoke(Span<const Tensor> i_operands, Span<const Tensor> i_attributes = {}, 
            const std::any & i_attachment = {} ) const;

    private:

        static TensorType DefaultDeduceType(const std::any & i_attachment,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes);

        const Overload * FindOverload(Span<const Tensor> i_operands) const;

        const Overload * FindOverloadWithPromotion(Span<const Tensor> i_operands) const;

    private:
        std::string const m_name;
        Flags m_flags = {};
        DeduceTypeFunction m_deduce_type_func = {};
        std::vector<Overload> m_overloads;
        SimplifyFunction m_simpily_func = {};
        GradientOfOperandFunction m_gradient_of_input_func = {};
    };
}
