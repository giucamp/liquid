
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "operator.h"

namespace liquid
{
    Operator::Operator(std::string_view i_name)
        : m_name(i_name)
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
        m_overloads.push_back(i_overload);
        return *this;
    }

    Operator & Operator::SetGradientOfOperand(GradientOfOperandFunction i_func)
    {
        m_gradient_of_input_func = i_func;
        return *this;
    }
}

