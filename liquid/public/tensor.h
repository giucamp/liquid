
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <type_traits>
#include "span.h"

namespace liquid
{
    template <typename TYPE>
        using EnableIfNumeric = std::enable_if_t<std::is_integral_v<TYPE> || std::is_floating_point_v<TYPE>>;

    class Tensor
    {
    public:

        /*Tensor(std::string_view i_name, ScalarType i_scalar_type = ScalarType::Any);

        template <typename SCALAR_TYPE, typename = EnableIfNumeric<SCALAR_TYPE>>
        Tensor()
        {

        }*/

    public:

        Tensor(std::shared_ptr<const class Expression> && i_expression)
            : m_expression(std::move(i_expression))
                { }

        const std::shared_ptr<const Expression> & GetExpression() const { return m_expression;}

    private:
        std::shared_ptr<const Expression> m_expression;
    };

    bool Always(const Tensor & i_bool_tensor);

    bool Never(const Tensor& i_bool_tensor);

    Tensor operator == (const Tensor & i_first, const Tensor& i_second);

    Tensor Add(Span<Tensor const> i_addends);

    Tensor operator + (const Tensor & i_operand);

    Tensor operator + (const Tensor & i_first, const Tensor& i_second);

    Tensor & operator += (Tensor & i_first, const Tensor& i_second);
}
