
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include <variant>
#include <optional>

namespace liquid
{
    namespace miu6
    {
        struct Token
        {
            enum class Kind
            {
                Unrecognized,
                EndOfSource,
            
                Name,
                Literal,
                Comma,

                // scalar types
                Any,
                Real,
                Bool,
                Integer,

                // arithmetic operators
                Plus,
                Minus,
                Times,
                Div,
                Pow,

                // comparison
                Equal,
                NotEqual,
                Less,
                Greater,
                LessOrEqual,
                GreaterOrEqual,

                // boolean operators
                And,
                Or,
                Not,

                // bracket 
                LeftParenthesis,
                RightParenthesis,
                LeftBracket,
                RightBracket,
                LeftBrace,
                RightBrace,

                // if
                If,
                Then,
                Elif,
                Else,
            };

            Kind m_kind = Kind::EndOfSource;
            std::variant<std::monostate, Real, Integer, Bool> m_value;
            std::string_view m_chars;
        };

        class Lexer
        {
        public:

            Lexer(std::string_view i_source);

            const Token & GetCurrentToken() const { return m_curr_token; }

            bool IsCurrentToken(Token::Kind i_token_kind) const
                { return GetCurrentToken().m_kind == i_token_kind; }

            void Advance();

            /* if the current token matches the specified kind, returns it and the advances */
            [[nodiscard]] std::optional<Token> TryAccept(Token::Kind i_token_kind);

            Token Accept(Token::Kind i_token_kind);

            friend std::ostream & operator << (std::ostream & i_dest, const Lexer & i_lexer);

        private:

            static bool TrySkipSpaces(std::string_view & io_source);
            static bool TryParseString(std::string_view & io_source, std::string_view i_what);
            static std::optional<Real> TryParseReal(std::string_view & io_source);
            static std::optional<Integer> TryParseInteger(std::string_view & io_source);
            static std::optional<Bool> TryParseBool(std::string_view & io_source);
            static std::optional<std::string_view> TryParseName(std::string_view & io_source);

            static Token TryParseToken(std::string_view & io_source);
            static Token TryParseTokenImpl(std::string_view & io_source);

        private:
            std::string_view m_whole_source;
            std::string_view m_remaining_source;
            Token m_curr_token;
        };

    } // namespace miu6

} // namespace liquid
