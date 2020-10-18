
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "scope.h"
#include "expression.h"
#include "book.h"
#include <algorithm>

namespace liquid
{
    namespace
    {
        // thank's to https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const/8147213#8147213
        class ConcreteRoot : public Scope
        {
        public:
            ConcreteRoot(const std::shared_ptr<const Scope> & i_parent)
                : Scope(i_parent) { }
        };
    }

    const std::shared_ptr<const Scope> & Scope::Root()
    {
        static const std::shared_ptr<const Scope> root = std::make_shared<ConcreteRoot>(nullptr);
        return root;
    }

    Scope::Scope(const std::shared_ptr<const Scope> & i_parent)
        : m_parent(i_parent)
    {
    }

    std::shared_ptr<Scope> Scope::MakeInner() const
    {
        return std::make_shared<ConcreteRoot>(shared_from_this());
    }

    void Scope::AddVariable(const Tensor & i_new_variable)
    {
        if(!IsVariable(i_new_variable))
            Panic("Scope::AddVariable", i_new_variable, " is not a variable");
        std::string const & name = i_new_variable.GetExpression()->GetName();
        if(!name.empty())
        {
            Member const existing = TryLookup(name);

            if(std::holds_alternative<Tensor>(existing))
                Panic("Scope::AddVariable - duplicate members:\n", 
                    std::get<Tensor>(existing), " and\n", i_new_variable);

            if(std::holds_alternative<std::reference_wrapper<const Operator>>(existing))
                Panic("Scope::AddVariable - duplicate members:\n operator ", 
                    std::get<std::reference_wrapper<const Operator>>(existing).get().GetName(),
                    " and\n", i_new_variable);
        }
            
        m_declarations.push_back(i_new_variable);
    }

    Scope::Member Scope::Lookup(std::string_view i_name) const
    {
        auto member = TryLookup(i_name);
        if(std::holds_alternative<std::monostate>(member))
            Panic("could not find ", i_name, " in this scope");
        return member;
    }

    Scope::Member Scope::TryLookup(std::string_view i_name) const
    {
        // try to find a variable
        auto const val_it = std::find_if(m_declarations.begin(), m_declarations.end(), 
            [i_name](const Tensor & i_candidate){
                return i_candidate.GetExpression()->GetName() == i_name; });
        if(val_it != m_declarations.end())
            return *val_it;

        if(m_parent != nullptr)
        {
            // try an inherited member
            auto const inherited_member = m_parent->TryLookup(i_name);
            if(!std::holds_alternative<std::monostate>(inherited_member))
                return inherited_member;
        }

        // try an operator - only the root scope does it
        else if(auto const op = Book::Get().TryGetOperator(i_name))
        {
            return std::ref(*op);
        }

        return {};
    }
}
