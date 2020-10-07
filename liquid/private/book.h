
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "operator.h"
#include "liquid/tensor.h"
#include <memory>

namespace liquid
{
    class Book
    {
    public:

        Book(Book &&) = delete;
        Book(const Book &) = delete;
        
        static Book & Get();

        using Element = std::variant<std::string_view, Tensor>;

        void AddParagraph(std::string_view i_title, Span<Element const> i_elements);

        template <typename... ELEMENT, typename = std::enable_if_t<
                (std::is_constructible_v<Element, ELEMENT> && ...) > >
            void AddParagraph(std::string_view i_topic, const ELEMENT& ... i_elements)
        {
            AddParagraph(i_topic, {i_elements...});
        }

        void AddProposition(const char * i_topic, const Tensor & i_bool_expression, const char * i_cpp_expression);

        void AddPanicProposition(const char * i_topic, const char * i_cpp_expression, const char * i_message);

        const Operator & GetOperator(const std::string & i_name) const;

        std::vector<std::string> GetAllTopics() const;

    private:
        Book();
        ~Book();

    private:
        struct Content;
        std::unique_ptr<Content> m_content;
    };
}
