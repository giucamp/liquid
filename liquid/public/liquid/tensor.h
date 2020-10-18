
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

namespace liquid
{
    template <typename TYPE>
        using EnableIfNumeric = std::enable_if_t<std::is_integral_v<TYPE> || std::is_floating_point_v<TYPE>>;

    class Tensor
    {
    public:

        Tensor(std::string_view i_miu6_code, ScalarType i_scalar_type = ScalarType::Any);

        Tensor(std::string_view i_miu6_code, Span<const Integer> i_shape, 
            ScalarType i_scalar_type = ScalarType::Any);

        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(const SCALAR & i_scalar)
                : Tensor(ScalarConst{}, CanonicalizeScalar(i_scalar)) { }

        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            Tensor(const SCALAR & i_scalar, Span<const Integer> i_shape)
                : Tensor(ScalarConst{}, CanonicalizeScalar(i_scalar), i_shape) { }

        Tensor(std::initializer_list<Tensor> i_vector);

        Tensor(std::initializer_list<Tensor> i_vector, Span<const Integer> i_shape);

        ScalarType GetScalarType() const;

    public:

        Tensor(const std::shared_ptr<const class Expression> & i_expression)
            : m_expression(i_expression) { }

        Tensor(std::shared_ptr<const class Expression> && i_expression)
            : m_expression(std::move(i_expression)) { }

        const std::shared_ptr<const Expression> & GetExpression() const { return m_expression; }

    private:
        enum class ScalarConst {};
        Tensor(ScalarConst, Real i_scalar);
        Tensor(ScalarConst, Integer i_scalar);
        Tensor(ScalarConst, Bool i_scalar);
        Tensor(ScalarConst, Real i_scalar, Span<const Integer> i_shape);
        Tensor(ScalarConst, Integer i_scalar, Span<const Integer> i_shape);
        Tensor(ScalarConst, Bool i_scalar, Span<const Integer> i_shape);
        
        template <typename TYPE>
            auto CanonicalizeScalar(TYPE i_scalar)
        {
            if constexpr(std::is_same_v<TYPE, bool>)
                return i_scalar;
            else if constexpr(std::is_floating_point_v<TYPE>)
                return NumericCast<Real>(i_scalar);
            else if constexpr(std::is_integral_v<TYPE>)
                return NumericCast<Integer>(i_scalar);
        }
            
    private:
        // expressions are immutable
        std::shared_ptr<const Expression> m_expression;
    };

    inline Tensor operator ""_miu6(const char * i_string, size_t i_length)
    {
        return Tensor(std::string_view{i_string, i_length});
    }

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
    Tensor operator / (const Tensor & i_dividend, const Tensor & i_divisor);

    Tensor operator && (const Tensor & i_first_bool, const Tensor & i_second_bool);
    Tensor operator || (const Tensor & i_first_bool, const Tensor & i_second_bool);
    Tensor operator ! (const Tensor & i_bool_operand);

    Tensor & operator += (Tensor & i_first, const Tensor & i_second);
    Tensor & operator -= (Tensor & i_first, const Tensor & i_second);
    Tensor & operator *= (Tensor & i_first, const Tensor & i_second);
    Tensor & operator /= (Tensor & i_dividend, const Tensor & i_divisor);

    Tensor operator < (const Tensor & i_first, const Tensor & i_second);
    Tensor operator > (const Tensor & i_first, const Tensor & i_second);
    Tensor operator <= (const Tensor & i_first, const Tensor & i_second);
    Tensor operator >= (const Tensor & i_first, const Tensor & i_second);
    Tensor operator == (const Tensor & i_first, const Tensor & i_second);
    Tensor operator != (const Tensor & i_first, const Tensor & i_second);

    Tensor Log(const Tensor & i_source);
    Tensor Exp(const Tensor & i_source);
    Tensor Pow(const Tensor & i_base, const Tensor & i_exponent);
    Tensor Square(const Tensor & i_source);
    Tensor Sin(const Tensor & i_operand);
    Tensor Cos(const Tensor & i_operand);

    Tensor Stack(Span<Tensor const> i_tensors);

    // to implement
    Tensor Substitute(const Tensor & i_where, const Tensor & i_what,
        const Tensor & i_with, const Tensor & i_when = true);

    struct Rule
    {
        Tensor const m_what, m_with, m_when = true;
    };

    Tensor Substitute(const Tensor & i_where, Span<const Rule> i_rules);

    std::ostream & operator << (std::ostream & i_dest, const Tensor & i_tensor);
}
