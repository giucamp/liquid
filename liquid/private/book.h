
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "operator.h"
#include "liquid/tensor.h"
#include <unordered_map>
#include <variant>

namespace liquid
{
    class Book
    {
    public:

        Book(Book &&) = delete;
        Book(const Book &) = delete;
        
        static Book & Get();

        using Element = std::variant<std::string_view, Tensor>;

        struct Paragraphs
        {
            std::vector<Element> m_elements;
        };

        struct Proposition
        {
            Tensor m_bool_expression;
            std::string m_cpp_source_code;
            std::string m_miu6_source_code;
        };

        struct PanicProposition
        {
            std::string m_panic_message;
            std::string m_cpp_source_code;
            std::string m_miu6_source_code;
        };

        void AddParagraph(std::string_view i_title, Span<Element const> i_elements);

        template <typename... ELEMENT, typename = std::enable_if_t<
                (std::is_constructible_v<Element, ELEMENT> && ...) > >
            void AddParagraph(std::string_view i_topic, const ELEMENT& ... i_elements)
        {
            AddParagraph(i_topic, {i_elements...});
        }

        void AddProposition(const char * i_topic, const Tensor & i_bool_expression, 
            const char * i_cpp_source_code, const char * i_miu6_source_code);

        void AddPanicProposition(const char * i_topic, const char * i_panic_message,
            const char * i_cpp_source_code, const char * i_miu6_source_code);

        const Operator & GetOperator(std::string_view i_name) const;

        const Operator * TryGetOperator(std::string_view i_name) const;

        std::vector<std::string> GetAllTopics() const;

    private:

        Book();

        void AddOperator(const Operator & i_operator);

    private:
        std::unordered_map<std::string, const Operator &> m_operators;
        std::unordered_map<std::string, const Paragraphs> m_paragraphs;
        std::unordered_multimap<std::string, const Proposition> m_propositions;
        std::unordered_multimap<std::string, const PanicProposition> m_panic_propositions;
    };
}
