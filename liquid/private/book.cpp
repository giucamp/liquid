
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "book.h"
#include "expression.h"
#include "operator.h"

namespace liquid
{
    extern const Operator & GetOperatorAdd();
    extern const Operator & GetOperatorAnd();
    extern const Operator & GetOperatorCast();
    extern const Operator & GetOperatorConstant();
    extern const Operator & GetOperatorEqual();
    extern const Operator & GetOperatorExp();
    extern const Operator & GetOperatorIf();
    extern const Operator & GetOperatorIs();
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

    void Book::AddOperator(const Operator & i_operator)
    {
        bool const inserted = m_operators.insert(std::make_pair(
            i_operator.GetName(), &i_operator)).second;
        if(!inserted)
            Panic("Book - operator ", i_operator.GetName(), " already exists");
    }

    const Operator * Book::TryGetOperator(std::string_view i_name) const
    {
        // sadly until C++20 we can't lookup with a string_view
        auto const it = m_operators.find(std::string(i_name));
        if(it == m_operators.end())
            return nullptr;
        else
            return it->second;
    }

    const Operator & Book::GetOperator(std::string_view i_name) const
    {
        if(auto const op = TryGetOperator(i_name))
            return *op;
        Panic("Book - operator ", i_name, " mot found");
    }

    void Book::AddParagraph(std::string_view i_topic, Span<Element const> i_elements)
    {
        Paragraphs topic {{i_elements.begin(), i_elements.end()}};
        bool const inserted = m_paragraphs.insert(std::make_pair(i_topic, std::move(topic))).second;
        if(!inserted)
            Panic("Book - paragraph ", i_topic, " already present");
    }

    std::vector<std::string> Book::GetAllTopics() const
    {
        return Transform(m_paragraphs, [](auto i_pair){ return i_pair.first; });
    }

    void Book::AddProposition(const char * i_topic, const Tensor & i_bool_expression,
        const char * i_cpp_source_code, const char * i_miu6_source_code)
    {
        Proposition proposition{ i_bool_expression, 
            i_cpp_source_code != nullptr ? i_cpp_source_code : "", 
            i_miu6_source_code != nullptr ? i_miu6_source_code : "" };
        m_propositions.insert(std::make_pair(i_topic, std::move(proposition)));
    }

    void Book::AddPanicProposition(const char * i_topic, const char * i_panic_message,
        const char * i_cpp_source_code, const char * i_miu6_source_code)
    {
        PanicProposition proposition{ i_panic_message,
            i_cpp_source_code != nullptr ? i_cpp_source_code : "", 
            i_miu6_source_code != nullptr ? i_miu6_source_code : "" };
        m_panic_propositions.insert(std::make_pair(i_topic, std::move(proposition)));
    }

    Book::Book()
    {
        AddOperator(GetOperatorConstant());
        AddOperator(GetOperatorEqual());
        AddOperator(GetOperatorExp());
        AddOperator(GetOperatorIf());
        AddOperator(GetOperatorIs());
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

    Book & Book::Get()
    {
        static Book s_instance;
        return s_instance;
    }
}
