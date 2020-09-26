
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "liquid/liquid_common.h"
#include "liquid/scalars_initializer.h"

namespace liquid
{
    int64_t TensorInitializer::GetRank() const
    {
        if (Is<TensorInitializer>())
            return std::get<std::vector<TensorInitializer>>(m_elements).at(0).GetRank() + 1;
        else
            return 1;
    }

    std::pair<std::vector<Integer>, ScalarType> TensorInitializer::GetShapeAndType() const
    {
        std::vector<Integer> shape(NumericCast<size_t>(GetRank()), -1);
        ScalarType type = ScalarType::Any;
        GetShapeAndTypeImpl(0, shape, type);
        return {shape, type};
    }

    Integer TensorInitializer::GetFirstDimension() const
    {
        auto const size = std::visit([](auto const& i_elements) { return i_elements.size(); }, m_elements);
        return NumericCast<Integer>(size);
    }

    void TensorInitializer::GetShapeAndTypeImpl(size_t i_curr_dim, Span<int64_t> o_shape, ScalarType& o_type) const
    {
        Integer const first_dim = GetFirstDimension();
        if (o_shape[i_curr_dim] < 0)
            o_shape[i_curr_dim] = first_dim;
        else if (o_shape[i_curr_dim] != first_dim)
            Panic("TensorInitializer - inconsistend ", i_curr_dim, "-th dimension: could be ",
                o_shape[i_curr_dim], " or ", first_dim);

        ScalarType this_type = ScalarType::Any;
        if (Is<Real>())
            this_type = ScalarType::Real;
        else if (Is<Integer>())
            this_type = ScalarType::Integer;
        else if (Is<Bool>())
            this_type = ScalarType::Bool;
        else if (Is<TensorInitializer>())
        {
            auto const& elements = std::get<std::vector<TensorInitializer>>(m_elements);
            for(const TensorInitializer & initializer : elements)
                initializer.GetShapeAndTypeImpl(i_curr_dim + 1, o_shape, o_type);
        }

        if (this_type != o_type)
        {
            if(o_type == ScalarType::Any)
                o_type = this_type;
            else
                Panic("TensorInitializer - Conflicting scalar types: ", 
                    this_type, " and ", o_type);
        }
    }
}
