
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "liquid/liquid_common.h"
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

        Operator(std::string_view i_name);

        Tensor Invoke(const Tensor & i_single_operand, Span<const Tensor> i_attributes = {},
            const std::any & i_attachment = {}) const;

        Tensor Invoke(Span<const Tensor> i_operands, Span<const Tensor> i_attributes = {},
            const std::any & i_attachment = {} ) const;

        Tensor Invoke(std::string_view i_name, std::string_view i_doc,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes = {},
            const std::any & i_attachment = {}) const;


                // flags
 
        enum class Flags
        {
            None = 0,
            Commutative = 1 << 0,
            Associative = 1 << 1
        };

        friend Flags operator | (Flags i_first, Flags i_second)
            { return static_cast<Flags>(static_cast<int>(i_first) | static_cast<int>(i_second)); }

        Operator & AddFlags(Flags i_flags);


                // type deduction

        using DeduceTypeFunction = TensorType(*)(const std::any & i_attachment, Span<const Tensor> i_operands, Span<const Tensor> i_attributes);

        Operator & SetDeduceType(DeduceTypeFunction i_func);
        

                // evaluation

        using EvaluateWithAttachmentFunction = TensorValue(*)(const std::any & i_attachment,
            const TensorType & i_result_type, Span<const TensorValue> i_operands,
            Span<const TensorValue> i_attributes);

        using EvaluateFromVariablesFunction = TensorValue(*)(const std::any & i_attachment,
            const TensorType & i_result_type, Span<const Tensor> i_operands,
            Span<const Tensor> i_attributes);

        using EvaluateFunction = TensorValue(*)(const TensorType & i_result_type,
            Span<const TensorValue> i_operands, Span<const TensorValue> i_attributes);

        using EvaluateNoAttributesFunction = TensorValue(*)(const TensorType & i_result_type,
            Span<const TensorValue> i_operands);

        using EvaluateSingleArgument = TensorValue(*)(const TensorType & i_result_type,
            const TensorValue & i_operand);

        using VarEvaluateFunction = std::variant<EvaluateFunction, EvaluateNoAttributesFunction, 
            EvaluateSingleArgument, EvaluateWithAttachmentFunction,
            EvaluateFromVariablesFunction>;

        struct Parameter
        {
            TensorType m_type;
            std::string m_name;
            std::optional<Tensor> m_default_value;
        };

        struct Overload
        {
            VarEvaluateFunction m_evaluate = {};
            std::vector<Parameter> m_parameters;
            size_t m_variadic_parameters_count = {};
        };

        Operator & AddOverload(const Overload& i_overload);

        Operator & AddOverload(VarEvaluateFunction i_evaluate, 
            std::vector<Parameter> i_parameters = {},
            size_t i_variadic_parameters_count = {})
        {
            return AddOverload({i_evaluate, std::move(i_parameters), i_variadic_parameters_count});
        }
        
            // canonicalization
        
        using CanonicalizeFunction = std::optional<Tensor>(*)(const Tensor & i_source);

        Operator & SetCanonicalize(CanonicalizeFunction i_func);


            // eligible for propagation

        using EligibleForPropagation = bool(*)(const std::any & i_attachment, Span<const Tensor> i_operands, Span<const Tensor> i_attributes);

        Operator & SetEligibleForPropagation(EligibleForPropagation i_func);


            // gradient of operand

        using GradientOfOperandFunction = Tensor(*)(const Tensor & i_self, const Tensor & i_self_gradient, size_t i_operand_index);

        Operator & SetGradientOfOperand(GradientOfOperandFunction i_func);


            // canonicalization

        Operator & SetIdentityElement(const TensorValue & i_value);

    private:

        static TensorType DefaultDeduceType(const std::any & i_attachment,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes);

        enum class OverloadMatchFlags
        {
            None = 0,
            AllowNumericPromotion = 1 << 0, /**< consider implicit numeric promotion 
                for the match */
            ProcessArguments = 1 << 1 /**< perform implicit numeric promotion 
                and default argument substitution */
        };

        friend OverloadMatchFlags operator | (OverloadMatchFlags i_first, OverloadMatchFlags i_second)
            { return CombineFlags(i_first, i_second); }

        bool IsEligibleForPropagation(const std::any & i_attachment, 
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes) const;

        static bool OverloadMatch(const Operator::Overload & i_overload, 
            Span<const Tensor> i_operands, OverloadMatchFlags i_flags,
            std::vector<Tensor> & o_arguments);

        const Overload * TryFindOverload(Span<const Tensor> i_operands,
            OverloadMatchFlags i_flags, std::vector<Tensor> & o_arguments) const;

        const Overload & FindOverload(Span<const Tensor> i_operands, 
            std::vector<Tensor> & o_arguments) const;

        std::optional<Tensor> TryConstantPropagation(
            const Overload & i_overload, const TensorType & i_result_type,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes,
            const std::any & i_attachment) const;

        TensorValue Evaluate(const Overload & i_overload, const TensorType & i_result_type,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes,
            const std::any & i_attachment) const;

        static std::vector<TensorValue> ToValues(Span<const Tensor> i_tensors);

    private:
        std::string const m_name;
        Flags m_flags = {};
        DeduceTypeFunction m_deduce_type_func = {};
        EligibleForPropagation m_eligible_for_propagation = {};
        std::vector<Overload> m_overloads = {};
        CanonicalizeFunction m_canonicalize_func = {};
        GradientOfOperandFunction m_gradient_of_input_func = {};
        std::optional<TensorValue> m_identity_element;
    };
}
