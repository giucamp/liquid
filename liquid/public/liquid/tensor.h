
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <type_traits>
#include <optional>
#include <any>
#include "liquid/liquid_common.h"
#include "liquid/span.h"
#include "liquid/tensor_initializer.h"

namespace liquid
{
    template <typename TYPE>
        using EnableIfNumeric = std::enable_if_t<std::is_integral_v<TYPE> || std::is_floating_point_v<TYPE>>;

    // https://timvieira.github.io/blog/post/2014/02/11/exp-normalize-trick/

    class Tensor
    {
    public:

        Tensor(const TensorInitializer & i_scalars);

        Tensor(const TensorInitializer & i_scalars, Span<const Integer> i_shape);

        Tensor(std::string_view i_liquid_code, ScalarType i_scalar_type = ScalarType::Any);

        Tensor(std::string_view i_liquid_code, Span<const Integer> i_shape, 
            ScalarType i_scalar_type = ScalarType::Any);

        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(const SCALAR & i_scalar)
                : Tensor(TensorInitializer({i_scalar}), Span<const Integer>{}) { }

        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(const SCALAR & i_scalar, Span<const Integer> i_shape)
                : Tensor(TensorInitializer({ i_scalar }), i_shape) { }

        // std::initializer_list - rank 1
        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(std::initializer_list<SCALAR> i_vector, Span<const Integer> i_shape)
                : Tensor(TensorInitializer(i_vector), i_shape) { }
        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(std::initializer_list<SCALAR> i_vector)
                : Tensor(TensorInitializer(i_vector)) { }

        // std::initializer_list - rank 2
        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(std::initializer_list<std::initializer_list<SCALAR>> i_matrix, Span<const Integer> i_shape)
                : Tensor(TensorInitializer(i_matrix), i_shape) { }
        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(std::initializer_list<std::initializer_list<SCALAR>> i_matrix)
                : Tensor(TensorInitializer(i_matrix)) { }

        ScalarType GetScalarType() const;

    public:

        Tensor(std::shared_ptr<const class Expression> && i_expression)
            : m_expression(std::move(i_expression)) { }

        const std::shared_ptr<const Expression> & GetExpression() const { return m_expression; }

    private:
        std::shared_ptr<const Expression> m_expression;
    };

    Tensor Rank(const Tensor & i_tensor);

    Tensor Shape(const Tensor & i_tensor);

    bool IsConstant(const Tensor & i_tensor);

    template <typename SCALAR_TYPE>
        Span<const SCALAR_TYPE> GetConstantStorage(const Tensor & i_tensor);

    template <typename SCALAR_TYPE>
        std::vector<SCALAR_TYPE> ConstantToVector(const Tensor & i_tensor);

    bool Always(const Tensor & i_bool_tensor);
    
    bool Never(const Tensor & i_bool_tensor);
    
    Tensor Add(Span<Tensor const> i_addends);

    template <typename... TENSOR, typename = std::enable_if_t<
            (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor Add(TENSOR && ... i_addends)
    {
        return Add({i_addends...});
    }

    Tensor Mul(Span<Tensor const> i_factors);

    template <typename... TENSOR, typename = std::enable_if_t<
            (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor Mul(TENSOR && ... i_factors)
    {
        return Mul({i_factors...});
    }

    Tensor If(Span<Tensor const> i_operands);

    template <typename... TENSOR, typename = std::enable_if_t<
            (std::is_constructible_v<Tensor, TENSOR> && ...) &&
            sizeof...(TENSOR) % 2 == 1 >>
        Tensor If(TENSOR && ... i_operands)
    {
        return If({i_operands...});
    }

    Tensor And(Span<Tensor const> i_bool_operands);
    
    template <typename... TENSOR, typename = std::enable_if_t<
            (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor And(TENSOR && ... i_bool_operands)
    {
        return And({i_bool_operands...});
    }

    Tensor Or(Span<Tensor const> i_bool_operands);

    template <typename... TENSOR, typename = std::enable_if_t<
            (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor Or(TENSOR && ... i_bool_operands)
    {
        return Or({i_bool_operands...});
    }

    Tensor Not(Tensor const & i_bool_operand);

    Tensor Cast(ScalarType i_dest_scalar_type, const Tensor & i_source);

    template <typename DEST_SCALAR_TYPE>
        Tensor Cast(const Tensor & i_source)
    {
        if constexpr(std::is_same_v<DEST_SCALAR_TYPE, Real>)
            return Cast(ScalarType::Real, i_source);
        else if constexpr (std::is_same_v<DEST_SCALAR_TYPE, Integer>)
            return Cast(ScalarType::Integer, i_source);
        else
            static_assert("DEST_SCALAR_TYPE must be Real or Integer");
    }

    Tensor operator + (const Tensor & i_operand);
    Tensor operator - (const Tensor & i_operand);

    Tensor operator + (const Tensor & i_first, const Tensor & i_second);
    Tensor operator - (const Tensor & i_first, const Tensor & i_second);
    Tensor operator * (const Tensor & i_first, const Tensor & i_second);

    Tensor operator && (const Tensor & i_first_bool, const Tensor & i_second_bool);
    Tensor operator || (const Tensor & i_first_bool, const Tensor & i_second_bool);
    Tensor operator ! (const Tensor & i_bool_operand);

    Tensor & operator += (Tensor & i_first, const Tensor & i_second);
    Tensor & operator -= (Tensor & i_first, const Tensor & i_second);
    Tensor & operator *= (Tensor & i_first, const Tensor & i_second);

    Tensor operator < (const Tensor & i_first, const Tensor & i_second);
    Tensor operator > (const Tensor & i_first, const Tensor & i_second);
    Tensor operator <= (const Tensor & i_first, const Tensor & i_second);
    Tensor operator >= (const Tensor & i_first, const Tensor & i_second);
    Tensor operator == (const Tensor & i_first, const Tensor & i_second);
    Tensor operator != (const Tensor & i_first, const Tensor & i_second);

    inline bool AssertCheck(const Tensor & i_value) { return Always(i_value); }
}
