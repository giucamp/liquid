
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "operator.h"
#include "expression.h"
#include <algorithm>

namespace liquid
{
    extern const Operator & GetOperatorConstant();

    Operator::Operator(std::string_view i_name)
        : m_name(i_name), m_deduce_type_func(DefaultDeduceType)
    {

    }

    Operator & Operator::SetDoc(std::string_view i_description, std::string_view i_return_type)
    {
        m_doc_description = i_description;
        m_doc_return_type = i_return_type;
        return *this;
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

    bool Operator::IsRegularNAry() const
    {
        if(!HasFlags(m_flags, Flags::Commutative | Flags::Associative))
            return false;
            
        for (const Overload & overload : m_overloads)
            if(overload.m_parameters.size() != 1 ||
                overload.m_variadic_parameters_count != 1)
                    return false;

        return true;
    }

    Operator & Operator::AddOverload(const Overload & i_overload)
    {
        if(i_overload.m_variadic_parameters_count > i_overload.m_parameters.size())
            Panic("Badformed overload");

        m_overloads.push_back(i_overload);
        return *this;
    }

    Operator & Operator::SetGradientOfOperand(GradientOfOperandFunction i_func)
    {
        m_gradient_of_input_func = i_func;
        return *this;
    }

    Operator & Operator::SetIdentityElement(const TensorValue & i_value)
    {
        m_identity_element = i_value;
        return *this;
    }

    TensorType Operator::DefaultDeduceType(
        [[maybe_unused]] const std::any & i_attachment,
        Span<const Tensor> i_operands)
    {
        std::vector<TensorType> types;
        types.reserve(i_operands.size());
        for(const Tensor & operand : i_operands)
            types.push_back(operand.GetExpression()->GetType());
        return DeduceType(types);
    }

    bool Operator::OverloadMatch(const Operator::Overload & i_overload, 
        Span<const Tensor> i_operands, OverloadMatchFlags i_flags,
        std::vector<Tensor> & o_arguments)
    {
        auto & parameters = i_overload.m_parameters;
        size_t const variadic_parameters = i_overload.m_variadic_parameters_count;
        size_t const non_variadic_parameters = parameters.size() - variadic_parameters;

        if(HasFlags(i_flags, OverloadMatchFlags::ProcessArguments))
        {
            o_arguments.clear();
            o_arguments.insert(o_arguments.begin(), i_operands.begin(), i_operands.end());
        }

        // check variadic repetitions
        size_t variadic_repetitions = 0;
        if (i_overload.m_variadic_parameters_count > 0)
        {
            size_t const variadic_arguments = i_operands.size() - non_variadic_parameters;
            variadic_repetitions = variadic_arguments / variadic_parameters;
            if((variadic_arguments % variadic_parameters) != 0)
                return false;
        }

        // check operands types
        size_t parameter_index = 0;
        size_t variadic_repetition_index = 0;
        for (size_t operand_index = 0; operand_index < i_operands.size(); operand_index++)
        {
            if(parameter_index >= variadic_parameters)
            {
                variadic_repetition_index++;
                parameter_index = 0;
            }

            if (variadic_repetition_index >= variadic_repetitions &&
                parameter_index < variadic_parameters)
            {
                parameter_index = variadic_parameters;
            }

            auto const & argument_type = i_operands[operand_index].GetExpression()->GetType();
            if (!parameters[parameter_index].m_type.IsSupercaseOf(argument_type))
            {
                // mismatching types
                bool const promotion_feasible =
                    HasFlags(i_flags, OverloadMatchFlags::AllowNumericPromotion) &&
                    parameters[parameter_index].m_type.GetScalarType() == ScalarType::Real &&
                    argument_type.GetScalarType() == ScalarType::Integer;

                if(!promotion_feasible)
                    return false;

                if(HasFlags(i_flags, OverloadMatchFlags::ProcessArguments))
                    o_arguments[operand_index] = Cast<Real>(o_arguments[operand_index]);
            }

            parameter_index++;
        }
        return true;
    }

    const Operator::Overload * Operator::TryFindOverload(Span<const Tensor> i_operands,
        OverloadMatchFlags i_flags, std::vector<Tensor> & o_arguments) const
    {
        for (const Overload & overload : m_overloads)
        {
            if(OverloadMatch(overload, i_operands, i_flags, o_arguments))
                return &overload;
        }
        return nullptr;
    }

    const Operator::Overload & Operator::FindOverload(Span<const Tensor> i_operands,
        std::vector<Tensor> & o_arguments) const
    {
        if (const Overload* overload = TryFindOverload(i_operands, 
                OverloadMatchFlags::None, o_arguments))
            return *overload;

        if (const Overload* overload = TryFindOverload(i_operands,
                OverloadMatchFlags::AllowNumericPromotion, o_arguments))
            return *overload;

        Panic(m_name, ": could not find an overload matching the argument types: ",
            Span(Transform(i_operands, [](auto & i_op){ return i_op.GetExpression()->GetType(); } )) );
    }

    Tensor Operator::Invoke(Span<const Tensor> i_operands,
        const std::any & i_attachment) const
    {
        return Invoke({}, {}, i_operands, i_attachment);
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
            Span<const Tensor> i_operands) const
    {
        if (m_eligible_for_propagation)
        {
            return m_eligible_for_propagation(i_attachment, i_operands);
        }
        else
        {
            return this != &GetOperatorConstant() &&
                std::all_of(i_operands.begin(), i_operands.end(), IsConstant);
        }
    }

    TensorValue Operator::Evaluate(const Overload & i_overload, const TensorType & i_result_type,
            Span<const Tensor> i_operands, const std::any & i_attachment) const
    {
        if (std::holds_alternative<EvaluateFromVariablesFunction>(i_overload.m_evaluate))
        {
            return std::get<EvaluateFromVariablesFunction>(i_overload.m_evaluate)
                (i_attachment, i_result_type, i_operands);
        }

        auto const operand_values = ToValues(i_operands);
        
        if (std::holds_alternative<EvaluateSingleArgument>(i_overload.m_evaluate))
        {
            if(operand_values.size() != 1)
                Panic(m_name, ": evaluate supports only one operand, ", operand_values.size(), " provided");

            return std::get<EvaluateSingleArgument>(i_overload.m_evaluate)
                (i_result_type, operand_values.at(0));
        }
        else if(std::holds_alternative<EvaluateFunction>(i_overload.m_evaluate))
        {
            return std::get<EvaluateFunction>(i_overload.m_evaluate)
                (i_result_type, operand_values);
        }
        else if (std::holds_alternative<EvaluateWithAttachmentFunction>(i_overload.m_evaluate))
        {
            return std::get<EvaluateWithAttachmentFunction>(i_overload.m_evaluate)
                (i_attachment, i_result_type, operand_values);
        }

        Panic("Operator - internal error - unhandled evaluate function type");
    }

    std::optional<Tensor> Operator::TryConstantPropagation(
        const Overload & i_overload, const TensorType & i_result_type,
        Span<const Tensor> i_operands, const std::any & i_attachment) const
    {
        if (IsEligibleForPropagation(i_attachment, i_operands))
        {
            return MakeConstant(Evaluate(i_overload, i_result_type, 
                i_operands, i_attachment));
        }
        return {};
    }

    Tensor Operator::Invoke(std::string_view i_name, std::string_view i_doc,
        Span<const Tensor> i_operands, const std::any & i_attachment) const
    {
        std::vector<Tensor> operands;
        const Overload & overload = FindOverload(i_operands, operands);
        OverloadMatch(overload, i_operands, 
            OverloadMatchFlags::AllowNumericPromotion | OverloadMatchFlags::ProcessArguments, operands);

        const TensorType type = m_deduce_type_func(i_attachment, operands);

        if(auto constant = TryConstantPropagation(overload, type, operands, i_attachment))
            return *constant;

        // common commutative-associative canonicalizations
        if(Has(Flags::Commutative | Flags::Associative))
        {
            // flatten: add(x... add(y...) z...) -> add(x... y... z...)
            for(size_t op_index = 0; op_index < operands.size(); op_index++)
            {
                const Tensor & operand = operands[op_index];
                if(operand.GetExpression()->OperatorIs(*this))
                {
                    operands = Concatenate(
                        Span(operands).subspan(0, op_index),
                        operand.GetExpression()->GetOperands(),
                        Span(operands).subspan(op_index + 1) );
                }
            }

            // sort operands by hash
            std::sort(operands.begin(), operands.end(), [](const Tensor & i_left, const Tensor & i_right)
                { return i_left.GetExpression()->GetHash() < i_right.GetExpression()->GetHash(); } );
        }

        Tensor result(std::make_shared<Expression>(
            i_name, i_doc, type, *this, overload, 
            operands, i_attachment ));

        if(m_canonicalize_func != nullptr)
        {
            if(auto simplified = m_canonicalize_func(result))
                result = *simplified;
        }

        return result;
    }

    Operator & Operator::SetAttachmentComparer(AttachmentComparer i_attachment_comparer)
    {
        m_attachment_comparer = i_attachment_comparer;
        return *this;
    }

    bool Operator::AttachmentsEqual(const std::any & i_left, const std::any & i_right) const
    {
        if(i_left.has_value() != i_right.has_value())
            return false;

        if(!i_left.has_value())
            return true;

        if(m_attachment_comparer == nullptr)
            Panic("Operator::AttachmentsEqual - ", m_name, ": no attachment comparer");

        return m_attachment_comparer(i_left, i_right);
    }

    Operator & Operator::SetAttachmentHasher(AttachmentHasher i_attachment_hasher)
    {
        m_attachment_hasher = i_attachment_hasher;
        return *this;
    }

    void Operator::HashAttachment(Hash & i_dest, const std::any & i_attachment) const
    {
        if(m_attachment_hasher == nullptr)
            Panic("Operator ", m_name, ": attachment hasher not set" );
        m_attachment_hasher(i_dest, i_attachment);
    }

    Hash & operator << (Hash & i_dest, const Operator & i_source)
    {
        i_dest << i_source.m_name;
        return i_dest;
    }
}

