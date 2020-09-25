
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "indices.h"

namespace liquid
{
    void Indices::DbgCheck()
    {
        #ifndef NDEBUG
            size_t const rank = m_shape.GetDimensions().size();
            Span<Integer const> strides = m_shape.GetStrides().subspan(1);
            Integer logical_linear_index = 0;
            for (size_t i = 0; i < rank; i++)
                logical_linear_index += m_indices[i] * strides[i];
            auto const logical_index_modulo = m_logical_linear_index% m_shape.GetLinearSize();
            if (logical_linear_index != logical_index_modulo)
                Panic("Indices - internal error: logical_linear_index != logical_index_modulo");
        #endif
    }

    Indices::Indices(const Shape& i_shape)
        : m_shape(i_shape), m_indices(i_shape.GetDimensions().size())
    {
        DbgCheck();
    }

    void Indices::Add(Integer i_addend)
    {
        m_logical_linear_index += i_addend;

        size_t dim_index = m_shape.GetDimensions().size();
        while (dim_index != 0)
        {
            dim_index--;

            m_indices[dim_index] += i_addend;

            Integer const this_dim = m_shape.GetDimension(dim_index);

            if(m_indices[dim_index] < this_dim)
                break;
            
            m_indices[dim_index] += this_dim;
            i_addend = m_indices[dim_index] / this_dim;
            m_indices[dim_index] %= this_dim;
        }

        DbgCheck();
    }

    void Indices::Increment()
    {
        m_logical_linear_index++;

        size_t dim_index = m_shape.GetDimensions().size();
        while (dim_index != 0)
        {
            dim_index--;

            m_indices[dim_index]++;

            Integer const this_dim = m_shape.GetDimension(dim_index);

            if (m_indices[dim_index] < this_dim)
                break;
            
            m_indices[dim_index] -= this_dim;
        }

        DbgCheck();
    }

    bool Indices::IsInBounds() const
    {
        return m_logical_linear_index < m_shape.GetLinearSize();
    }
}
