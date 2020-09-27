
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <variant>
#include <type_traits>
#include <utility>
#include "liquid/liquid_common.h"
#include "liquid/span.h"

namespace liquid
{
    class TensorInitializer
    {
    public:

        template <typename ELEMENT_TYPE>
            TensorInitializer(std::initializer_list<ELEMENT_TYPE> i_list)
        {
            if(i_list.size() == 0)
                Panic("TensorInitializer - zero dimension");

            if constexpr (std::is_floating_point_v<ELEMENT_TYPE>)
            {
                std::vector<Real> scalars(i_list.size());
                for(size_t i = 0; i < i_list.size(); i++)
                    scalars[i] = static_cast<Real>(i_list.begin()[i]);
                m_elements = std::move(scalars);
            }
            else if constexpr (std::is_same_v<ELEMENT_TYPE, Bool>)
            {
                std::vector<Bool> scalars(i_list.size());
                for (size_t i = 0; i < i_list.size(); i++)
                    scalars[i] = i_list.begin()[i];
                m_elements = std::move(scalars);
            }
            else if constexpr (std::is_integral_v<ELEMENT_TYPE>)
            {
                std::vector<Integer> scalars(i_list.size());
                for (size_t i = 0; i < i_list.size(); i++)
                    scalars[i] = NumericCast<Integer>(i_list.begin()[i]);
                m_elements = std::move(scalars);
            }
            else
            {
                std::vector<TensorInitializer> elements;
                elements.reserve(i_list.size());
                for (size_t i = 0; i < i_list.size(); i++)
                    elements.emplace_back<TensorInitializer>(i_list.begin()[i]);
                m_elements = std::move(elements);
            }
        }

        TensorInitializer(TensorInitializer &&) = default;
        TensorInitializer & operator = (TensorInitializer&&) = default;

        int64_t GetRank() const;

        std::pair<std::vector<Integer>, ScalarType> GetShapeAndType() const;

        Integer GetFirstDimension() const;

        template <typename ELEMENT_TYPE>
            bool Is() const
        { 
            return std::holds_alternative<std::vector<ELEMENT_TYPE>>(m_elements); 
        }

        template <typename ELEMENT_TYPE>
            ELEMENT_TYPE At(Span<const Integer> i_indices) const
        {
            if(i_indices.size() != static_cast<size_t>(GetRank()))
                Panic("TensorInitializer - wrong number of indices");

            const TensorInitializer * curr = this;
            while (i_indices.size() > 1)
            {
                auto const & elements = std::get<std::vector<TensorInitializer>>(m_elements);
                curr = &elements.at(NumericCast<size_t>(i_indices[0]));
                i_indices = i_indices.subspan(1);
            }

            if (!curr->Is<ELEMENT_TYPE>())
                Panic("TensorInitializer - wrong scalar type");

            auto const & elements = std::get<std::vector<ELEMENT_TYPE>>(curr->m_elements);
            return elements.at(NumericCast<size_t>(i_indices[0]));
        }

    private:

        void GetShapeAndTypeImpl(size_t i_curr_dim, Span<int64_t> o_shape, ScalarType & o_type) const;

    private:
        std::variant<
            std::vector<Real>,
            std::vector<Integer>,
            std::vector<Bool>,
            std::vector<TensorInitializer>
        > m_elements;
    };
}
