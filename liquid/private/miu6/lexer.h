
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "miu6/alphabet.h"
#include <variant>
#include <optional>

namespace liquid
{
    namespace miu6
    {
        // a token is a range of chars recognized by the lexer
        struct Token
        {
            SymbolId m_symbol_id = SymbolId::EndOfSource;
            std::variant<std::monostate, Real, Integer, Bool> m_value;
            SymbolFlags m_flags = SymbolFlags::None;
            int32_t m_precedence = -1000;
            std::string_view m_chars;
        };

        class Lexer
        {
        public:

            Lexer(std::string_view i_source);

            const Token & GetCurrentToken() const { return m_curr_token; }

            bool IsCurrentToken(SymbolId i_symbol_id) const
                { return GetCurrentToken().m_symbol_id == i_symbol_id; }

            bool IsSourceOver() const { return m_curr_token.m_symbol_id == SymbolId::EndOfSource; }

            void Advance();

            /* if the current token matches the specified kind, returns it and the advances */
            std::optional<Token> TryAccept(SymbolId i_symbol_id);

            Token Accept(SymbolId i_symbol_id);

            friend std::ostream & operator << (std::ostream & i_dest, const Lexer & i_lexer);

        private:

            static std::string_view TryParseSpaces(std::string_view & io_source);
            static bool TryParseString(std::string_view & io_source, std::string_view i_what);
            static std::optional<Real> TryParseReal(std::string_view & io_source);
            static std::optional<Integer> TryParseInteger(std::string_view & io_source);
            static std::optional<Bool> TryParseBool(std::string_view & io_source);
            static std::optional<std::string_view> TryParseName(std::string_view & io_source);
            static bool WhiteSimmetry(std::string_view i_prefix, std::string_view i_postfix);

            static Token TryParseToken(std::string_view & io_source);
            static Token TryParseTokenImpl(std::string_view i_prefix_spaces, std::string_view & io_source);

        private:
            Token m_curr_token;
            std::string_view m_remaining_source;
            std::string_view m_whole_source;
        };

    } // namespace miu6

} // namespace liquid
