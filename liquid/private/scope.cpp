
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
            // for some reason inherited constructors don't work here...
            ConcreteRoot(const std::shared_ptr<const Scope> & i_parent,
                Span<const Rule> i_rules, Span<const Tensor> i_values)
                    : Scope(i_parent, i_rules, i_values) { }
        };
    }

    const std::shared_ptr<const Scope> & Scope::Root()
    {
        static const std::shared_ptr<const Scope> root = std::make_shared<ConcreteRoot>(
            nullptr, Span<const Rule>{}, Span<const Tensor>{});
        return root;
    }

    Scope::Scope(const std::shared_ptr<const Scope> & i_parent,
        Span<const Rule> i_rules, Span<const Tensor> i_values)
            : m_parent(i_parent), 
              m_rules(i_rules.begin(), i_rules.end()),
              m_values(i_values.begin(), i_values.end())
    {
        
    }

    std::shared_ptr<const Scope> Scope::MakeInner(Span<const Rule> i_rules, Span<const Tensor> i_values) const
    {
        return std::make_shared<ConcreteRoot>(shared_from_this(), i_rules, i_values);
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
        // try to find a value
        auto const val_it = std::find_if(m_values.begin(), m_values.end(), 
            [i_name](const Tensor & i_candidate){
                return i_candidate.GetExpression()->GetName() == i_name; });
        if(val_it != m_values.end())
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
