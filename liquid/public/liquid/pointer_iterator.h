
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace liquid
{
    template <typename IT_TYPE> class PointerIterator
    {
    public:

        using value_type = IT_TYPE;
        using reference = IT_TYPE&;
        using pointer = IT_TYPE*;
        using const_reference = const IT_TYPE&;
        using difference_type = ptrdiff_t;
        using size_type = size_t;
        using iterator_category = std::random_access_iterator_tag;

        explicit PointerIterator(IT_TYPE* i_pointer)
            : m_pointer(i_pointer) {}

        bool operator == (const PointerIterator& i_other) const { return m_pointer == i_other.m_pointer; }
        bool operator != (const PointerIterator& i_other) const { return m_pointer != i_other.m_pointer; }
        bool operator > (const PointerIterator& i_other) const { return m_pointer > i_other.m_pointer; }
        bool operator >= (const PointerIterator& i_other) const { return m_pointer >= i_other.m_pointer; }
        bool operator < (const PointerIterator& i_other) const { return m_pointer < i_other.m_pointer; }
        bool operator <= (const PointerIterator& i_other) const { return m_pointer <= i_other.m_pointer; }

        PointerIterator& operator ++ () { m_pointer++; return *this; }
        PointerIterator operator ++ (int) { PointerIterator copy = *this; copy.m_pointer++; return copy; }
        PointerIterator& operator -- () { m_pointer--; return *this; }
        PointerIterator operator -- (int) { PointerIterator copy = *this; copy.m_pointer--; return copy; }

        PointerIterator& operator += (ptrdiff_t i_delta) { m_pointer += i_delta; return *this; }
        PointerIterator& operator -= (ptrdiff_t i_delta) { m_pointer -= i_delta; return *this; }

        reference operator * () const { return *m_pointer; }
        pointer operator -> () const { return m_pointer; }
        reference operator[](size_type i_index) const { return m_pointer[i_index]; }

        friend difference_type operator - (const PointerIterator& i_first, const PointerIterator& i_second)
        {
            return i_first.m_pointer - i_second.m_pointer;
        }

        template <typename INTEGER, typename = std::enable_if_t<std::is_integral_v<INTEGER> && !std::is_same_v<INTEGER, bool>>>
            friend PointerIterator operator + (const PointerIterator& i_it, INTEGER i_offset)
        {
            return PointerIterator{ i_it.m_pointer + i_offset };
        }

        template <typename INTEGER, typename = std::enable_if_t<std::is_integral_v<INTEGER> && !std::is_same_v<INTEGER, bool>>>
            friend PointerIterator operator + (INTEGER i_offset, const PointerIterator& i_it)
        {
            return PointerIterator{ i_it.m_pointer + i_offset };
        }

    private:
        IT_TYPE* m_pointer{};
    };

    // IsContainerOfV<CONTAINER, ELEMENT_TYPE>
    template <typename CONTAINER, typename ELEMENT_TYPE, typename = std::void_t<>>
        struct IsContainerOf : std::false_type { };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        struct IsContainerOf < CONTAINER, ELEMENT_TYPE, std::void_t<

            decltype(std::begin(std::declval<CONTAINER>()) != std::end(std::declval<CONTAINER>())),

            std::enable_if_t<std::is_same_v<
                std::decay_t<ELEMENT_TYPE>,
                std::decay_t<decltype(*std::begin(std::declval<CONTAINER>()))>
            >>

    > > : std::true_type{ };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        constexpr bool IsContainerOfV = IsContainerOf<CONTAINER, ELEMENT_TYPE>::value;

    // IsContigousContainerOfV<CONTAINER, ELEMENT_TYPE>
    template <typename CONTAINER, typename ELEMENT_TYPE, typename = std::void_t<>>
        struct IsContigousContainerOf : std::false_type { };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        struct IsContigousContainerOf< CONTAINER, ELEMENT_TYPE, std::void_t< std::enable_if_t<std::conjunction_v<

            // decltype(std::data(i_source_container)) must be convertible to ELEMENT_TYPE *
            std::is_convertible<decltype(std::data(std::declval<CONTAINER>())), ELEMENT_TYPE*>,

            // decltype(std::size(i_source_container)) must be convertible to size_t
            std::is_convertible<decltype(std::size(std::declval<CONTAINER>())), size_t>,

            // decltype(std::data())(*)[] must be convertible to ELEMENT_TYPE (*)[]
            std::is_convertible<std::remove_pointer_t<decltype(std::data(std::declval<CONTAINER>()))>(*)[], ELEMENT_TYPE(*)[]>

    > > > > : std::true_type { };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        constexpr bool IsContigousContainerOfV = IsContigousContainerOf<CONTAINER, ELEMENT_TYPE>::value;


    template <typename CONTAINER, typename = std::void_t<>>
        struct ContainerElementType { };
    template <typename CONTAINER>
        struct ContainerElementType<CONTAINER, std::void_t<
        std::decay_t<decltype(*std::begin(std::declval<CONTAINER>()))> > >
    { 
        using type = std::decay_t<decltype(*std::begin(std::declval<CONTAINER>()))>;
    };

    template <typename CONTAINER>
        using ContainerElementTypeT = typename ContainerElementType<CONTAINER>::type;
}
