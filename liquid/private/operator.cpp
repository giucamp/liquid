
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "operator.h"
#include "expression.h"

namespace liquid
{
    extern const Operator & GetOperatorConstant();

    Operator::Operator(std::string_view i_name)
        : m_name(i_name), m_deduce_type_func(DefaultDeduceType)
    {

    }

    Operator & Operator::SetDeduceType(DeduceTypeFunction i_func)
    {
        if(i_func == nullptr)
            Panic("Operator::SetDeduceType - null function");
        m_deduce_type_func = i_func;
        return *this;
    }

    Operator & Operator::SetCanonicalize(CanonicalizeFunction i_func)
    {
        m_canonicalize_func = i_func;
        return *this;
    }

    Operator & Operator::SetEligibleForPropagation(EligibleForPropagation i_func)
    {
        m_eligible_for_propagation = i_func;
        return *this;
    }

    Operator & Operator::AddFlags(Flags i_flags)
    {
        m_flags = m_flags | i_flags;
        return *this;
    }

    Operator & Operator::AddOverload(const Overload & i_overload)
    {
        if(i_overload.m_variadic_parameters_count > i_overload.m_parameter_types.size())
            Panic("Badformed overload");

        m_overloads.push_back(i_overload);
        return *this;
    }

    Operator & Operator::SetGradientOfOperand(GradientOfOperandFunction i_func)
    {
        m_gradient_of_input_func = i_func;
        return *this;
    }

    TensorType Operator::DefaultDeduceType([[maybe_unused]] const std::any & i_attachment,
        Span<const Tensor> i_operands, [[maybe_unused]] Span<const Tensor> i_attributes)
    {
        std::vector<TensorType> types;
        types.reserve(i_operands.size());
        for(const Tensor & operand : i_operands)
            types.push_back(operand.GetExpression()->GetType());
        return DeduceType(types);
    }

    bool Operator::OverloadMatch(const Operator::Overload & i_overload, 
        Span<const Tensor> i_operands, OverloadMatchFlags i_flags)
    {
        auto & parameters = i_overload.m_parameter_types;
        size_t const variadic_parameters = i_overload.m_variadic_parameters_count;
        size_t const non_variadic_parameters = parameters.size() - variadic_parameters;

        // check variadic repetitions
        size_t variadic_repetitions = 0;
        if (i_overload.m_variadic_parameters_count > 0)
        {
            size_t const variadic_arguments = parameters.size() - non_variadic_parameters;
            variadic_repetitions = variadic_arguments / variadic_parameters;
            if((variadic_arguments % variadic_parameters) != 0)
                return false;
        }

        // check operands types
        size_t parameter_index = 0;
        size_t variadic_repetition_index = 0;
        for (size_t operand_index = 0; operand_index < parameters.size(); operand_index++)
        {
            while(parameter_index >= i_overload.m_parameter_types.size())
            {
                if (variadic_repetition_index >= variadic_repetitions)
                    return false;

                variadic_repetition_index++;
                parameter_index = 0;
            }

            auto const & argument_type = i_operands[operand_index].GetExpression()->GetType();
            if (!parameters[parameter_index].IsSupercaseOf(argument_type))
            {
                // mismatching types
                bool const promotion_feasible =
                    i_flags == OverloadMatchFlags::AllowNumericPromotion &&
                    parameters[parameter_index].GetScalarType() == ScalarType::Real &&
                    argument_type.GetScalarType() == ScalarType::Integer;

                if(!promotion_feasible)
                    return false;
            }
        }
        return true;
    }

    const Operator::Overload * Operator::TryFindOverload(Span<const Tensor> i_operands,
        OverloadMatchFlags i_flags) const
    {
        const Operator::Overload * match = nullptr;
        int matches = 0;
        for (const Overload & overload : m_overloads)
        {
            if(OverloadMatch(overload, i_operands, i_flags))
            {
                matches++;
                match = &overload;
            }
        }
        return matches == 1 ? match : nullptr;
    }

    const Operator::Overload & Operator::FindOverload(Span<const Tensor> i_operands) const
    {
        if (const Overload* overload = TryFindOverload(i_operands, OverloadMatchFlags::None))
            return *overload;

        if (const Overload* overload = TryFindOverload(i_operands, OverloadMatchFlags::AllowNumericPromotion))
            return *overload;

        Panic(m_name, ": could not find a matching overload");
    }

    Tensor Operator::Invoke(Span<const Tensor> i_operands, Span<const Tensor> i_attributes,
        const std::any& i_attachment) const
    {
        return Invoke({}, {}, i_operands, i_attributes, i_attachment);
    }

    Tensor Operator::Invoke(const Tensor & i_single_operand, Span<const Tensor> i_attributes,
        const std::any & i_attachment) const
    {
        return Invoke({}, {}, { i_single_operand }, i_attributes, i_attachment);
    }

    std::vector<TensorValue> Operator::ToValues(Span<const Tensor> i_tensors)
    {
        std::vector<TensorValue> values;
        values.reserve(i_tensors.size());
        for(auto const & tensor : i_tensors)
            values.push_back(GetConstantValue(tensor));
        return values;
    }

    bool Operator::IsEligibleForPropagation(const std::any & i_attachment,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes) const
    {
        if (m_eligible_for_propagation)
        {
            return m_eligible_for_propagation(i_attachment, i_operands, i_attributes);
        }
        else
        {
            return this != &GetOperatorConstant() &&
                std::all_of(i_operands.begin(), i_operands.end(), IsConstant) &&
                std::all_of(i_attributes.begin(), i_attributes.end(), IsConstant);
        }
    }

    TensorValue Operator::Evaluate(const Overload & i_overload, const TensorType & i_result_type,
            Span<const Tensor> i_operands, Span<const Tensor> i_attributes,
            const std::any & i_attachment) const
    {
        if (std::holds_alternative<EvaluateFromVariablesFunction>(i_overload.m_evaluate))
        {
            return std::get<EvaluateFromVariablesFunction>(i_overload.m_evaluate)
                (i_attachment, i_result_type, i_operands, i_attributes);
        }

        auto const operand_values = ToValues(i_operands);
        auto const attribute_values = ToValues(i_attributes);

        if (std::holds_alternative<EvaluateSingleArgument>(i_overload.m_evaluate))
        {
            if(operand_values.size() != 1)
                Panic(m_name, ": evaluate supports only one operand, ", operand_values.size(), " provided");

            return std::get<EvaluateSingleArgument>(i_overload.m_evaluate)
                (i_result_type, operand_values.at(0));
        }
        else if (std::holds_alternative<EvaluateNoAttributesFunction>(i_overload.m_evaluate))
        {
            if (attribute_values.size() != 0)
                Panic(m_name, ": evaluate dos not supports attributes, ", attribute_values.size(), " provided");

            return std::get<EvaluateNoAttributesFunction>(i_overload.m_evaluate)
                (i_result_type, operand_values);
        }
        else if(std::holds_alternative<EvaluateFunction>(i_overload.m_evaluate))
        {
            return std::get<EvaluateFunction>(i_overload.m_evaluate)
                (i_result_type, operand_values, attribute_values);
        }
        else if (std::holds_alternative<EvaluateWithAttachmentFunction>(i_overload.m_evaluate))
        {
            return std::get<EvaluateWithAttachmentFunction>(i_overload.m_evaluate)
                (i_attachment, i_result_type, operand_values, attribute_values);
        }
    }

    std::optional<Tensor> Operator::TryConstantPropagation(
        const Overload & i_overload, const TensorType & i_result_type,
        Span<const Tensor> i_operands, Span<const Tensor> i_attributes, 
        const std::any & i_attachment) const
    {
        if (IsEligibleForPropagation(i_attachment, i_operands, i_attributes))
        {
            return MakeConstant(Evaluate(i_overload, i_result_type, 
                i_operands, i_attributes, i_attachment));
        }
        return {};
    }

    Tensor Operator::Invoke(std::string_view i_name, std::string_view i_doc,
        Span<const Tensor> i_operands, Span<const Tensor> i_attributes,
        const std::any& i_attachment) const
    {
        const Overload & overload = FindOverload(i_operands);

        const TensorType type = m_deduce_type_func(i_attachment, i_operands, i_attributes);

        if(auto constant = TryConstantPropagation(overload, type, i_operands, i_attributes, i_attachment))
            return *constant;

        Tensor result(std::make_shared<Expression>(
            i_name, i_doc, type, *this, overload, 
            i_operands, i_attributes, i_attachment ));

        if(m_canonicalize_func != nullptr)
        {
            if(auto simplified = m_canonicalize_func(result))
                result = *simplified;
        }

        return result;
    }
}

