
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <assert.h>
#include <vector>
#include "liquid/pointer_iterator.h"

namespace liquid
{
    template <typename TYPE>
        class Span
    {
    public:

        using value_type = TYPE;
        using reference = TYPE&;
        using pointer = TYPE*;
        using const_reference = const TYPE&;
        using difference_type = ptrdiff_t;
        using size_type = size_t;
        using iterator = PointerIterator<TYPE>;
        using const_iterator = PointerIterator<const TYPE>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        Span() = default;

        Span(TYPE * i_data, size_t i_size)
            : m_data(i_data), m_size(i_size) { }

        template <typename SOURCE_CONTAINER, typename = std::enable_if_t<IsContigousContainerOfV<SOURCE_CONTAINER, TYPE>>>
            Span(SOURCE_CONTAINER && i_source_container)
                : m_data(std::data(i_source_container)),
                  m_size(std::size(i_source_container))
                    { }

        Span(std::initializer_list<TYPE> && i_initializer_list)
            : m_data(i_initializer_list.begin()),
              m_size(i_initializer_list.size())
                { }

        bool empty() const { return m_size == 0; }

        TYPE * data() const { return m_data; }

        size_t size() const { return m_size; }

        const TYPE & operator[](size_t i_index) const
        {
            assert(i_index < m_size);
            return m_data[i_index];
        }

        TYPE & operator[](size_t i_index)
        {
            assert(i_index < m_size);
            return m_data[i_index];
        }

        const TYPE & at(size_t i_index) const
        {
            if(i_index >= m_size)
                throw std::out_of_range("Span - out of range");
            return m_data[i_index];
        }

        TYPE & at(size_t i_index)
        {
            if(i_index >= m_size)
                throw std::out_of_range("Span - out of range");
            return m_data[i_index];
        }

        TYPE & front()
        {
            assert(m_size > 0);
            return m_data[0];
        }

        const TYPE & front() const
        {
            assert(m_size > 0);
            return m_data[0];
        }

        TYPE & back()
        {
            assert(m_size > 0);
            return m_data[m_size - 1];
        }

        const TYPE & back() const
        {
            assert(m_size > 0);
            return m_data[m_size - 1];
        }

        Span subspan(size_t i_offset, size_t i_size) const
        {
            assert(i_offset <= m_size && i_offset + i_size <= m_size);
            return {m_data + i_offset, i_size};
        }

        Span subspan(size_t i_offset) const
        {
            return subspan(i_offset, m_size - i_offset);
        }

        bool content_equals(Span<TYPE> i_other) const
        {
            return m_size == i_other.m_size &&
                std::equal(begin(), end(), i_other.begin());
        }

        iterator begin() { return iterator{ m_data }; }
        const_iterator begin() const { return const_iterator{ m_data }; }
        const_iterator cbegin() const { return const_iterator{ m_data }; }
        reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
        const_reverse_iterator crbegin() const { return std::make_reverse_iterator(end()); }

        iterator end() { return iterator{ m_data + m_size }; }
        const_iterator end() const { return const_iterator{ m_data + m_size }; }
        const_iterator cend() const { return const_iterator{ m_data + m_size }; }
        reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator crend() const { return std::make_reverse_iterator(begin()); }

    private:
        TYPE * m_data{};
        size_t m_size{};
    };

    template <typename TYPE>
        std::ostream & operator << (std::ostream & i_ostream, Span<TYPE> i_span)
    {
        for (size_t i = 0; i < i_span.size(); i++)
        {
            if(i != 0)
                i_ostream << ", ";
            i_ostream << i_span[i];
        }
        return i_ostream;
    }

    template <typename SOURCE_CONTAINER>
        auto ToSpan(SOURCE_CONTAINER && i_source_container)
    {
        using ElementType = std::remove_reference_t<decltype(*std::begin(i_source_container))>;
        return Span<ElementType>(i_source_container);
    }

    template <typename SOURCE_CONTAINER, typename PREDICATE>
        auto Transform(const SOURCE_CONTAINER & i_source_elements, const PREDICATE & i_predicate)
    {
        using DestType = decltype(i_predicate(*std::declval<SOURCE_CONTAINER>().begin()));
        std::vector<DestType> result;
        auto const size = std::size(i_source_elements);
        result.reserve(size);
        for(const auto & source : i_source_elements)
            result.push_back(i_predicate(source));
        return result;
    }
}
