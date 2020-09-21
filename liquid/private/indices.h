
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <vector>
#include "liquid.h"
#include "shape.h"

namespace liquid
{
    class Indices
    {
    public:

        Indices(const Shape & i_shape);

        Indices & operator ++ ()
        {
            Increment();
            return *this;
        }

        void operator ++ (int)
        {
            Increment();
        }

        Indices& operator += (Integer i_addend)
        {
            Add(i_addend);
            return *this;
        }

        explicit operator bool() const
        {
            return IsInBounds();
        }

        template <typename SCALAR_TYPE>
            SCALAR_TYPE & operator[](Span<SCALAR_TYPE> i_scalars)
        {
            Integer const physical_linear_index = m_shape.GetPhysicalLinearIndex(m_indices);
            return i_scalars[static_cast<size_t>(physical_linear_index)];
        }

        template <typename CONTAINER>
            typename CONTAINER::value_type & operator[](CONTAINER & i_scalars)
        {
            return operator[](Span<typename CONTAINER::value_type>(i_scalars));
        }

        void Increment();

        void Add(Integer i_addend);

        bool IsInBounds() const;

        Integer GetLogicalLinearIndex() const { return m_logical_linear_index; }

        Span<const Integer> GetIndices() const { return m_indices; }

    private: 
        void DbgCheck();

    private:
        Shape m_shape;
        std::vector<Integer> m_indices;
        Integer m_logical_linear_index = 0;
    };

} // namespace liquid
