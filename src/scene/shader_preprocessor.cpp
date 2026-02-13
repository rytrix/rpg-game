#include "shader_preprocessor.hpp"
namespace {

constexpr bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool is_num(char c)
{
    return c >= '0' && c <= '9';
}

constexpr bool is_anum(char c)
{
    return is_alpha(c) || is_num(c);
}

constexpr bool is_whitespace(char c)
{
    return c == '\n' || c == ' ' || c == '\t' || c == '\r';
}

} // Anonymous namespace

std::string get_lines_between_delims_inclusive(std::string_view string, std::string_view start, std::string_view end)
{
    std::string result;
    std::vector<std::string_view> lines;
    usize begin = 0;
    for (usize i = 0; i < string.size(); i++) {
        if (string[i] == '\n') {
            lines.emplace_back(&string.at(begin), i - begin);
            begin = i + 1;
        } else if (i == string.size() - 1) {
            lines.emplace_back(&string.at(begin), i - begin);
        }
    }

    begin = 0;
    bool matched_start = false;
    for (usize i = 0; i < lines.size(); i++) {
        if (lines[i] == start) {
            begin = i;
            matched_start = true;
        }
        if (lines[i] == end) {
            util_assert(matched_start == true, std::format("Failed to match \"{}\"", start));
            for (; begin <= i; begin++) {
                result += lines[begin];
                result += '\n';
            }
            return result;
        }
    }

    return result;
}

std::string get_lines_between_delims(std::string_view string, std::string_view start, std::string_view end)
{
    std::string result;
    std::vector<std::string_view> lines;
    usize begin = 0;
    for (usize i = 0; i < string.size(); i++) {
        if (string[i] == '\n') {
            lines.emplace_back(&string.at(begin), i - begin);
            begin = i + 1;
        } else if (i == string.size() - 1) {
            lines.emplace_back(&string.at(begin), i - begin);
        }
    }

    begin = 0;
    bool matched_start = false;
    for (usize i = 0; i < lines.size(); i++) {
        if (lines[i] == start) {
            begin = i + 1;
            matched_start = true;
        }
        if (lines[i] == end) {
            for (; begin < i; begin++) {
                result += lines[begin];
                result += '\n';
            }
            util_assert(matched_start == true, std::format("Failed to match \"{}\"", start));
            return result;
        }
    }

    return result;
}

ShaderPreprocessor::ShaderPreprocessor(std::string_view text, std::initializer_list<std::string_view> conditions)
    : m_text(text)
    , m_conditions(conditions)
{
    parse_tokens();
}

void ShaderPreprocessor::parse_tokens()
{
    usize start_ptr = m_text_ptr;

    while (!at_end(m_text_ptr)) {
        if (at(m_text_ptr) == '%') {
            usize i = 1;
            while (m_text_ptr - i >= 0 && at(m_text_ptr - i) != '\n') {
                i++;
            }
            i--;
            m_tokens.emplace_back(
                Token::Text,
                std::string_view {
                    m_text.data() + start_ptr,
                    m_text_ptr - i - start_ptr });

            start_ptr = m_text_ptr;
            m_text_ptr++;

            while (!is_whitespace(at(m_text_ptr))) {
                m_text_ptr++;
            }

            std::string_view preprocess_text = { m_text.data() + start_ptr, m_text_ptr - start_ptr };
            // std::println("Preprocess text \"{}\"", preprocess_text);
            if (preprocess_text == "%Preprocess%") {
                m_text_ptr++;
                start_ptr = m_text_ptr;
                while (is_alpha(at(m_text_ptr))) {
                    m_text_ptr++;
                }
                std::string_view text = { m_text.data() + start_ptr, m_text_ptr - start_ptr };
                // std::println("Token text \"{}\"", text);
                if (text == "if") {
                    m_tokens.emplace_back(Token::If, text);

                    m_text_ptr++;
                    start_ptr = m_text_ptr;
                    while (is_anum(at(m_text_ptr))) {
                        m_text_ptr++;
                    }
                    text = { m_text.data() + start_ptr, m_text_ptr - start_ptr };
                    m_tokens.emplace_back(Token::Variable, text);
                    start_ptr = ++m_text_ptr;
                } else if (text == "endif") {
                    m_tokens.emplace_back(Token::EndIf, text);
                    start_ptr = m_text_ptr++;
                } else {
                    util_error("Malformed %Preprocess%");
                }
            } else {
                util_error("Malformed %Preprocess%");
            }
        }

        m_text_ptr++;
    }

    m_tokens.emplace_back(Token::Text, std::string_view { m_text.data() + start_ptr, m_text_ptr - start_ptr });
}

std::string ShaderPreprocessor::process()
{
    std::string result;
    for (usize i = 0; i < m_tokens.size(); i++) {
        if (m_tokens[i].token == Token::Text) {
            result += m_tokens[i].view;
        }
        if (m_tokens[i].token == Token::If) {
            i++;
            util_assert(m_tokens[i].token == Token::Variable, "ShaderPreprocessor no variable after if");
            bool include_text = false;
            for (const auto& condition : m_conditions) {
                if (m_tokens[i].view == condition) {
                    include_text = true;
                }
            }

            i++;
            if (include_text) {
                result += m_tokens[i].view;
            }
            i++;
            util_assert(m_tokens[i].token == Token::EndIf, "ShaderPreprocessor no EndIf after if text");
        }

        // std::println("{} \"{}\"", token_to_text(m_tokens[i].token), m_tokens[i].view);
    }
    return result;
}

const char* ShaderPreprocessor::token_to_text(Token token)
{
    switch (token) {
        case Token::Text:
            return "Text";
        case Token::If:
            return "If";
        case Token::EndIf:
            return "EndIf";
        case Token::Variable:
            return "Variable";
        case Token::Eof:
            return "Eof";
    };
}

char ShaderPreprocessor::at(usize index)
{
    util_assert(at_end(index) == false, "ShaderPreprocessor attempting to access out of m_text size");
    return m_text[index];
}

bool ShaderPreprocessor::at_end(usize index)
{
    return index >= m_text.size();
}
