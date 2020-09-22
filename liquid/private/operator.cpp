
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "operator.h"
#include "expression.h"

namespace liquid
{
    Operator::Operator(std::string_view i_name)
        : m_name(i_name), m_deduce_type_func(DefaultDeduceType)
    {

    }

    Operator & Operator::SetDeduceType(DeduceTypeFunction i_func)
    {
        m_deduce_type_func = i_func;
        return *this;
    }

    Operator & Operator::SetSimplify(const SimplifyFunction& i_func)
    {
        m_simpily_func = i_func;
        return *this;
    }

    Operator & Operator::AddFlags(Flags i_flags)
    {
        m_flags = m_flags | i_flags;
        return *this;
    }

    Operator & Operator::AddOverload(const Overload& i_overload)
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

    /*const Operator::Overload * Operator::FindOverload(Span<const Tensor> i_operands) const
    {
        const Operator::Overload * match = nullptr;
        for (const Overload& overload : m_overloads)
        {
            bool matching = true;
            size_t parameter_index = 0;
            for (size_t operand_index = 0; operand_index < overload.m_parameter_types.size(); operand_index++)
            {
                if (parameter_index >= overload.m_parameter_types.size())
                {
                    if(overload.m_variadic_parameters_count == 0)
                    {
                        matching = false;
                        break;
                    }
                }

                if (i_operands[operand_index].GetExpression()->GetType() !=
                    overload.m_parameter_types[operand_index])
                {
                    matching = false;
                    break;
                }
            }
            if(matching)
            {
                if(match != nullptr)
                    return nullptr;
                match = &overload;
            }
        }
    }

    const Operator::Overload * Operator::FindOverloadWithPromotion(Span<const Tensor> i_operands) const
    {

    }

    Tensor Operator::Invoke(Span<const Tensor> i_operands, Span<const Tensor> i_attributes,
        const std::any & i_attachment) const
    {

    }*/
}

