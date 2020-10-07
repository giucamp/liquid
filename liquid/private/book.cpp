
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "book.h"
#include "expression.h"
#include "operator.h"
#include <unordered_map>
#include <variant>

namespace liquid
{
    extern const Operator & GetOperatorAdd();
    extern const Operator & GetOperatorAnd();
    extern const Operator & GetOperatorCast();
    extern const Operator & GetOperatorConstant();
    extern const Operator & GetOperatorEqual();
    extern const Operator & GetOperatorExp();
    extern const Operator & GetOperatorIf();
    extern const Operator & GetOperatorLess();
    extern const Operator & GetOperatorLog();
    extern const Operator & GetOperatorMul();
    extern const Operator & GetOperatorNot();
    extern const Operator & GetOperatorOr();
    extern const Operator & GetOperatorPow();
    extern const Operator & GetOperatorRank();
    extern const Operator & GetOperatorShape();
    extern const Operator & GetOperatorStack();
    extern const Operator & GetOperatorVariable();

    struct Book::Content
    {
        struct Paragraphs
        {
            std::vector<Element> m_elements;
        };

        struct Proposition
        {
            Tensor m_bool_expression;
            std::string m_cpp_code;
            std::string m_miu6_code;
        };

        struct PanicProposition
        {
            std::string m_cpp_code;
            std::string m_panic_message;
        };

        std::unordered_map<std::string, const Operator &> m_operators;
        std::unordered_map<std::string, const Paragraphs> m_paragraphs;
        std::unordered_multimap<std::string, const Proposition> m_propositions;
        std::unordered_multimap<std::string, const PanicProposition> m_panic_propositions;

        void AddOperator(const Operator & i_operator)
        {
            bool const inserted = m_operators.insert(std::make_pair(i_operator.GetName(), i_operator)).second;
            if(!inserted)
                Panic("Book - operator ", i_operator.GetName(), " already exists");
        }
        
        const Operator & GetOperator(const std::string & i_name) const
        {
            auto const it = m_operators.find(i_name);
            if(it == m_operators.end())
                Panic("Book - operator ", i_name, " mot found");
            return it->second;
        }

        void AddParagraph(std::string_view i_topic, Span<Element const> i_elements)
        {
            Paragraphs topic {{i_elements.begin(), i_elements.end()}};
            bool const inserted = m_paragraphs.insert(std::make_pair(i_topic, std::move(topic))).second;
            if(!inserted)
                Panic("Book - paragraph ", i_topic, " already present");
        }

        std::vector<std::string> GetAllTopics() const
        {
            return Transform(m_paragraphs, [](auto i_pair){ return i_pair.first; });
        }

        void AddProposition(const char * i_topic, const Tensor & i_bool_expression, const char * i_cpp_expression)
        {
            Proposition proposition{i_bool_expression, i_cpp_expression, ToString(i_bool_expression) };
            m_propositions.insert(std::make_pair(i_topic, std::move(proposition)));
        }

        void AddPanicProposition(const char * i_topic, const char * i_cpp_expression, const char * i_panic_message)
        {
            PanicProposition proposition{i_cpp_expression, i_panic_message };
            m_panic_propositions.insert(std::make_pair(i_topic, std::move(proposition)));
        }

        Content()
        {
            AddOperator(GetOperatorConstant());
            AddOperator(GetOperatorEqual());
            AddOperator(GetOperatorExp());
            AddOperator(GetOperatorIf());
            AddOperator(GetOperatorLess());
            AddOperator(GetOperatorLog());
            AddOperator(GetOperatorMul());
            AddOperator(GetOperatorNot());
            AddOperator(GetOperatorOr());
            AddOperator(GetOperatorPow());
            AddOperator(GetOperatorRank());
            AddOperator(GetOperatorShape());
            AddOperator(GetOperatorStack());
            AddOperator(GetOperatorVariable());
        }
    };

    void Book::AddParagraph(std::string_view i_topic, Span<Element const> i_elements)
    {
        m_content->AddParagraph(i_topic, i_elements);
    }

    void Book::AddProposition(const char * i_topic, const Tensor & i_bool_expression, const char * i_cpp_expression)
    {
        m_content->AddProposition(i_topic, i_bool_expression, i_cpp_expression);
    }

    void Book::AddPanicProposition(const char * i_topic, const char * i_cpp_expression, const char * i_message)
    {
        m_content->AddPanicProposition(i_topic, i_cpp_expression, i_message);
    }

    std::vector<std::string> Book::GetAllTopics() const
    {
        return m_content->GetAllTopics();
    }

    const Operator & Book::GetOperator(const std::string & i_name) const
    {
        return m_content->GetOperator(i_name);
    }

    Book::Book()
        : m_content(std::make_unique<Content>())
            { }

    Book::~Book() = default;

    Book & Book::Get()
    {
        static Book s_instance;
        return s_instance;
    }
}
