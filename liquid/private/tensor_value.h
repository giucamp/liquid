
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include "liquid/liquid_common.h"
#include "liquid/span.h"
#include "liquid/tensor_initializer.h"
#include "shared_array.h"
#include "tensor_type.h"

namespace liquid
{
    class TensorValue
    {
    public:

        TensorValue(SharedArray<const Real> && i_reals, const Shape & i_shape);

        TensorValue(SharedArray<const Integer> && i_integers, const Shape & i_shape);

        TensorValue(SharedArray<const Bool> && i_bools, const Shape & i_shape);

        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            TensorValue(const SCALAR & i_scalar)
                : TensorValue(TensorInitializer({ i_scalar }), Shape({})) { }

        template <typename SCALAR, typename = EnableIfNumeric<SCALAR>>
            TensorValue(const SCALAR & i_scalar, const Shape & i_shape)
                : TensorValue(TensorInitializer({ i_scalar }), i_shape) { }

        TensorValue(const TensorInitializer & i_scalars);

        TensorValue(const TensorInitializer & i_scalars, const Shape & i_shape);

        const TensorType & GetType() const { return m_type; }

        const Shape & GetShape() const { return m_type.GetFixedShape(); }

        ScalarType GetScalarType() const { return m_type.GetScalarType(); }

        template <typename SCALAR_TYPE> bool Is() const
        {
            return std::holds_alternative<const SCALAR_TYPE>>(m_scalars);
        }

        template <typename SCALAR_TYPE>
            SharedArray<const SCALAR_TYPE> const & GetAs() const
        {
            return std::get<SharedArray<const SCALAR_TYPE>>(m_scalars);
        }

        template <typename SCALAR_TYPE>
            SharedArray<const SCALAR_TYPE> const & TryGetAs() const
        {
            if(Is<SCALAR_TYPE>())
                return GetAs<SCALAR_TYPE>();
            else
                return {};
        }

        friend bool operator == (const TensorValue & i_first, const TensorValue& i_second);

        friend bool operator != (const TensorValue& i_first, const TensorValue& i_second)
        {
            return !(i_first == i_second);
        }

    private:

        void SetFromInitializer(const TensorInitializer& i_scalars);

        template <typename SCALAR_TYPE>
            static size_t ConstantReduction(const Shape & i_shape, Span<const SCALAR_TYPE> i_scalars);

        void UntypedShapedReduction();

        void UnflattenLowerDim(const Shape & i_dest_shape);

        void Strip1UpperDims(const Shape& i_dest_shape);

    private:
        TensorType m_type;
        std::variant<
            SharedArray<const Real>,
            SharedArray<const Integer>,
            SharedArray<const Bool>
        > m_scalars;
    };
}
