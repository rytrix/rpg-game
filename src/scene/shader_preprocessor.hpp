#pragma once

std::string get_lines_between_delims_inclusive(std::string_view string, std::string_view start, std::string_view end);
std::string get_lines_between_delims(std::string_view string, std::string_view start, std::string_view end);

class ShaderPreprocessor : public NoCopyNoMove {
public:
    explicit ShaderPreprocessor(std::string_view text, std::initializer_list<std::string_view> conditions);

    std::string process();

private:
    enum class Token {
        Text,
        If,
        EndIf,
        Variable,
        Eof
    };

    static const char* token_to_text(Token token);

    struct TokenData {
        Token token;
        std::string_view view;

        TokenData(Token token, std::string_view view)
            : token(token)
            , view(view)
        {
        }
    };

    void parse_tokens();

    bool at_end(usize index);
    char at(usize index);

    usize m_text_ptr = 0;
    std::string_view m_text;
    std::initializer_list<std::string_view> m_conditions;

    std::vector<TokenData> m_tokens;
};