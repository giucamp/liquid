
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6/lexer.h"
#include <cctype>
#include <cstdlib>
#include <cinttypes>

namespace liquid
{
    namespace miu6
    {
        bool Lexer::TrySkipSpaces(std::string_view & io_source)
        {
            bool some_spaces = false;
            while(!io_source.empty() && std::isspace(io_source[0]))
            {
                io_source = io_source.substr(1);
                some_spaces = true;
            }
            return some_spaces;
        }

        bool Lexer::TryParseString(std::string_view & io_source, std::string_view i_what)
        {
            if (io_source.length() >= i_what.length() &&
                io_source.substr(0, i_what.length()) == i_what)
            {
                io_source = io_source.substr(i_what.length());
                return true;
            }
            else
                return false;
        }

        std::optional<Real> Lexer::TryParseReal(std::string_view & io_source)
        {
            static_assert(std::is_same_v<Real, double>, "std::strtod works with doubles");
            char * end_ptr = nullptr;
            auto const value = std::strtod(io_source.data(), &end_ptr);
            if(end_ptr == io_source.data())
                return {};
        
            io_source = io_source.substr(end_ptr - io_source.data());
            return value;
        }

        std::optional<Integer> Lexer::TryParseInteger(std::string_view & io_source)
        {
            char * end_ptr = nullptr;
            auto const value = std::strtoimax(io_source.data(), &end_ptr, 10);
            if(end_ptr == io_source.data())
                return {};

            io_source = io_source.substr(end_ptr - io_source.data());
            return NumericCast<Integer>(value);
        }

        std::optional<Bool> Lexer::TryParseBool(std::string_view & io_source)
        {
            if(TryParseString(io_source, "true"))
                return true;
            else if(TryParseString(io_source, "false"))
                return false;
            else
                return {};
        }

        std::optional<std::string_view> Lexer::TryParseName(std::string_view & io_source)
        {
            size_t length = 0;
            if (length < io_source.size() 
                && ( std::isalpha(io_source[length]) || io_source[length] == '_') )
            {
                do {
                    length++;
                } while(length < io_source.size()
                    && (std::isalnum(io_source[length]) || io_source[length] == '_') );
                auto name = io_source.substr(0, length);
                io_source = io_source.substr(length);
                return name;
            }
            else
                return {};
        }

        Token Lexer::TryParseToken(std::string_view & io_source)
        {
            TrySkipSpaces(io_source);

            const char * first_char = io_source.data();
            Token token = TryParseTokenImpl(io_source);
            token.m_chars = { first_char, static_cast<size_t>(io_source.data() - first_char) };
            return token;
        }

        Token Lexer::TryParseTokenImpl(std::string_view & io_source)
        {
            // scalar types
            if(TryParseString(io_source, "any"))
                return { Token::Kind::Any };
            else if(TryParseString(io_source, "real"))
                return { Token::Kind::Real };
            else if(TryParseString(io_source, "int"))
                return { Token::Kind::Integer };
            else if(TryParseString(io_source, "bool"))
                return { Token::Kind::Bool };

            // comparison
            else if(TryParseString(io_source, "=="))
                return { Token::Kind::Equal };
            else if(TryParseString(io_source, "!="))
                return { Token::Kind::NotEqual };
            else if(TryParseString(io_source, ">="))
                return { Token::Kind::GreaterOrEqual };
            else if(TryParseString(io_source, "<="))
                return { Token::Kind::LessOrEqual };
            else if(TryParseString(io_source, ">"))
                return { Token::Kind::Greater};
            else if(TryParseString(io_source, "<"))
                return { Token::Kind::Less };

            // boolean operators
            else if(TryParseString(io_source, "&&"))
                return { Token::Kind::And };
            else if(TryParseString(io_source, "||"))
                return { Token::Kind::Or };
            else if(TryParseString(io_source, "!"))
                return { Token::Kind::Not };

            // arithmetic operators
            else if(TryParseString(io_source, "+"))
                return { Token::Kind::Plus };
            else if(TryParseString(io_source, "-"))
                return { Token::Kind::Minus };
            else if(TryParseString(io_source, "*"))
                return { Token::Kind::Times };
            else if(TryParseString(io_source, "/"))
                return { Token::Kind::Div };
            else if(TryParseString(io_source, "^"))
                return { Token::Kind::Pow };

            // bracket
            else if(TryParseString(io_source, "("))
                return { Token::Kind::LeftParenthesis };
            else if(TryParseString(io_source, ")"))
                return { Token::Kind::RightParenthesis };
            else if(TryParseString(io_source, "["))
                return { Token::Kind::LeftBracket };
            else if(TryParseString(io_source, "]"))
                return { Token::Kind::RightBracket };
            else if(TryParseString(io_source, "{"))
                return { Token::Kind::LeftBrace };
            else if(TryParseString(io_source, "}"))
                return { Token::Kind::RightBrace };

            // if
            else if(TryParseString(io_source, "if"))
                return { Token::Kind::If };
            else if(TryParseString(io_source, "then"))
                return { Token::Kind::Then };
            else if(TryParseString(io_source, "elif"))
                return { Token::Kind::Elif };
            else if(TryParseString(io_source, "else"))
                return { Token::Kind::Else };

            // literals
            else if(auto const literal = TryParseBool(io_source))
                return { Token::Kind::Literal, *literal };
            else if(auto const literal = TryParseReal(io_source))
                return { Token::Kind::Literal, *literal };
            else if(auto const literal = TryParseInteger(io_source))
                return { Token::Kind::Literal, *literal };

            // misc
            else if(auto const name = TryParseName(io_source))
                return { Token::Kind::Name };
            else if(TryParseString(io_source, ","))
                return { Token::Kind::Comma };

            // end
            else if(io_source.empty())
                return { Token::Kind::EndOfSource };

            // unrecognized
            else
                return { Token::Kind::Unrecognized };
        }

        Lexer::Lexer(std::string_view i_source)
            : m_whole_source(i_source), m_remaining_source(i_source)
        {
            m_curr_token = TryParseToken(m_remaining_source);
        }

        void Lexer::Advance()
        {
            m_curr_token = TryParseToken(m_remaining_source);
        }

        std::optional<Token> Lexer::TryAccept(Token::Kind i_token_kind)
        {
            if (m_curr_token.m_kind == i_token_kind)
            {
                auto const result = m_curr_token;
                Advance();
                return result;
            }
            else
                return {};
        }

        Token Lexer::Accept(Token::Kind i_token_kind)
        {
            auto result = TryAccept(i_token_kind);
            if(result)
                return *result;
            Panic(*this, " Unexpected token");
        }

        struct Line { std::string_view m_chars; size_t m_number; };

        Line GetLineAt(std::string_view i_source, const char * i_at)
        {
            const char * curr_char = i_source.data();
            const char * const end_of_source = curr_char + i_source.size();

            if(i_at < curr_char || i_at > end_of_source)
                Panic("GetLineAt - i_at is outside the source code");

            const char * beginning_of_line = curr_char;
            size_t line_number = 1;
            while(curr_char < i_at)
            {
                if(*curr_char == '\n')
                {
                    line_number++;
                    curr_char++;
                    beginning_of_line = curr_char;
                }
                else if(*curr_char == '\r' &&
                    curr_char + 1 < end_of_source &&
                    curr_char[1] == '\n')
                {
                    line_number++;
                    curr_char += 2;
                    beginning_of_line = curr_char;
                }
                else
                    curr_char++;
            }

            const char * end_of_line = curr_char;
            while(end_of_line < end_of_source && *end_of_line != '\n')
                end_of_line++;
            if(end_of_line > i_source.data() && end_of_line[-1] == '\r')
                end_of_line--;

            return {std::string_view(beginning_of_line, end_of_line - beginning_of_line), line_number};
        }

        std::ostream & operator << (std::ostream & i_dest, const Lexer & i_lexer)
        {
            const char * at = i_lexer.m_curr_token.m_chars.data();
            Line const line = GetLineAt(i_lexer.m_whole_source, at);
            
            std::string const prefix = "(" + std::to_string(line.m_number) + "): ";
            i_dest << "\n" << prefix << line.m_chars;
            i_dest << "\n" << std::string(prefix.size() + (at - line.m_chars.data()), ' ') << '^';

            return i_dest;
        }

    } // namespace miu6
}
