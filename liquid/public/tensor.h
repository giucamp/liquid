
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <type_traits>
#include <optional>
#include <any>
#include "span.h"

namespace liquid
{
    template <typename TYPE>
        using EnableIfNumeric = std::enable_if_t<std::is_integral_v<TYPE> || std::is_floating_point_v<TYPE>>;

    // https://timvieira.github.io/blog/post/2014/02/11/exp-normalize-trick/

    class Tensor
    {
    public:


        template <typename SCALARS, typename = EnableIfNumeric<SCALARS>>
            Tensor(const SCALARS & i_scalars, const std::optional<Span<Integer>> & i_shape = {})
                : Tensor(CostructConstant{}, i_scalars, i_shape) { }

    public:

        Tensor(std::shared_ptr<const class Expression> && i_expression)
            : m_expression(std::move(i_expression))
                { }

        const std::shared_ptr<const Expression> & GetExpression() const { return m_expression;}

    private:

        enum class CostructConstant {};

        Tensor(CostructConstant, const std::any& i_scalars,
            const std::optional<Span<Integer>>& i_shape);

        enum class CostructExpression {};

        Tensor(CostructExpression, std::string_view i_expression, 
            const std::optional<Span<Integer>> & i_shape,
            ScalarType i_scalar_type);

    private:
        std::shared_ptr<const Expression> m_expression;
    };

    bool Always(const Tensor & i_bool_tensor);
    inline bool Always(bool i_bool_tensor) { return i_bool_tensor; }
    
    bool Never(const Tensor& i_bool_tensor);
    inline bool Never(bool i_bool_tensor) { return !i_bool_tensor; }

    Tensor operator == (const Tensor & i_first, const Tensor & i_second);

    Tensor Add(Span<Tensor const> i_addends);

    Tensor Cast(ScalarType i_dest_scalar_type, const Tensor & i_source);

    template <typename DEST_SCALAR_TYPE>
        Tensor Cast(const Tensor& i_source)
    {
        if constexpr(std::is_same_v<DEST_SCALAR_TYPE, Real>)
            return Cast(ScalarType::Real, i_source);
        else if constexpr (std::is_same_v<DEST_SCALAR_TYPE, Integer>)
            return Cast(ScalarType::Integer, i_source);
        else
            static_assert("DEST_SCALAR_TYPE must be Real or Integer");
    }

    Tensor operator + (const Tensor & i_operand);

    Tensor operator + (const Tensor & i_first, const Tensor& i_second);

    Tensor & operator += (Tensor & i_first, const Tensor& i_second);
}
