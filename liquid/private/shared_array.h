
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include "liquid/liquid_common.h"
#include "liquid/pointer_iterator.h"
#include "liquid/span.h"

namespace liquid
{
    template <typename TYPE>
        class SharedArray
    {
    public:

        using value_type = TYPE;
        using reference = TYPE&;
        using pointer = TYPE*;
        using const_pointer = const TYPE*;
        using const_reference = const TYPE&;
        using difference_type = ptrdiff_t;
        using size_type = size_t;
        using iterator = PointerIterator<TYPE>;
        using const_iterator = PointerIterator<const TYPE>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        SharedArray() = default;

        SharedArray(const SharedArray & i_source) = default;

        SharedArray(SharedArray && i_source) noexcept
            : m_elements(std::move(i_source.m_elements)), m_size(i_source.m_size)
        {
            i_source.m_size = 0;
        }

        friend void swap(SharedArray & i_first, SharedArray & i_second)
        {
            std::swap(i_first.m_elements, i_second.m_elements);
            std::swap(i_first.m_size, i_second.m_size);
        }

        SharedArray & operator = (SharedArray i_source)
        {
            swap(*this, i_source);
            return *this;
        }

        SharedArray(size_t i_size)
            : m_elements(new TYPE[i_size]()), m_size(i_size)
        {

        }

        SharedArray(std::initializer_list<TYPE> i_initializer_list)
            : m_elements(new TYPE[i_initializer_list.size()]), m_size(i_initializer_list.size())
        {
            for(size_t i = 0; i < m_size; i++)
                m_elements[i] = i_initializer_list.begin()[i];
        }

        template <typename SOURCE_CONTAINER,
            typename = std::enable_if_t<IsContainerV<SOURCE_CONTAINER, TYPE>>
        >
            SharedArray(const SOURCE_CONTAINER & i_source)
        {
            m_size = NumericCast<size_t>(std::distance(i_source.begin(), i_source.end()));

            auto const elements = new std::remove_const_t<TYPE>[m_size];
            m_elements = std::shared_ptr<TYPE[]>(elements);
            size_t index = 0;
            for(const auto & source_element : i_source)
                elements[index++] = source_element;
        }

        bool empty() const { return m_size == 0; }
        size_t size() const { return m_size; }
        pointer data() { return m_elements.get(); }
        const_pointer data() const { return m_elements.get(); }

        bool operator == (const SharedArray & i_other) const
        {
            return m_size == i_other.m_size &&
                std::equal(begin(), end(), i_other.begin());
        }

        bool operator != (const SharedArray& i_other) const { return !(*this == i_other); }

        iterator begin() { return iterator{ m_elements.get() }; }
        const_iterator begin() const { return const_iterator{ m_elements.get() }; }
        const_iterator cbegin() const { return const_iterator{ m_elements.get() }; }
        reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
        const_reverse_iterator crbegin() const { return std::make_reverse_iterator(end()); }

        iterator end() { return iterator{ m_elements.get() + m_size }; }
        const_iterator end() const { return const_iterator{ m_elements.get() + m_size }; }
        const_iterator cend() const { return const_iterator{ m_elements.get() + m_size }; }
        reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator crend() const { return std::make_reverse_iterator(begin()); }

        const TYPE & operator[](size_t i_index) const
        {
            return at(i_index);
        }

        TYPE & operator[](size_t i_index)
        {
            return at(i_index);
        }

        const TYPE & at(size_t i_index) const
        {
            LIQUID_ASSERT(i_index < m_size);
            return m_elements.get()[i_index];
        }

        TYPE & at(size_t i_index)
        {
            LIQUID_ASSERT(i_index < m_size);
            return m_elements.get()[i_index];
        }

        [[nodiscard]] SharedArray<TYPE> Resize(size_t i_new_size) const
        {
            if (i_new_size <= m_size)
                return Span<TYPE>(begin(), begin() + i_new_size);
            else
            {
                auto const new_elements = new std::remove_const_t<TYPE>[i_new_size];
                SharedArray<TYPE> result;
                result.m_elements = new_elements;
                result.m_size = i_new_size;
                for (size_t i = 0; i < m_size; i++)
                    new_elements[i] = *(m_elements.get() + i);
                return result;
            }
        }

    private:
        std::shared_ptr<TYPE[]> m_elements;
        size_t m_size{};
    };

} // namespace liquid
