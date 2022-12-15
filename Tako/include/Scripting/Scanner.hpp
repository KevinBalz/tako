#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "Utility.hpp"

namespace tako::Scripting
{
	// Scanner
	enum class TokenType
	{
		// Character Tokens
		LeftParen,
		RightParen,
		LeftBrace,
		RightBrace,
		Comma,
		Dot,
		Minus,
		Plus,
		Semicolon,
		Slash,
		Star,
		Bang,
		BangEqual,
		Equal,
		EqualEqual,
		Greater,
		GreaterEqual,
		Less,
		LessEqual,
		// Literals
		Identifier,
		String,
		Number,
		// Keywords
		And,
		Class,
		Else,
		True,
		False,
		Fun,
		For,
		If,
		Nil,
		Or,
		Print,
		Return,
		Var,
		While,

		Eof
	};

	inline static const std::unordered_map<std::string_view, TokenType> keywordsMap =
	{
		{"and", TokenType::And },
		{"class", TokenType::Class },
		{"else", TokenType::Else },
		{"true", TokenType::True },
		{"false", TokenType::False },
		{"fun", TokenType::Fun },
		{"For", TokenType::For },
		{"if", TokenType::If },
		{"nil", TokenType::Nil },
		{"or", TokenType::Or },
		//{"print", TokenType::Print },
		{"return", TokenType::Return },
		{"var", TokenType::Var },
		{"while", TokenType::While },
	};

	struct Token
	{
		TokenType type;
		std::string_view lexeme;
		int line;

		Token(TokenType type, std::string_view lexeme, int line) :
			type(type),
			lexeme(lexeme),
			line(line)
		{
		}
	};

	class Scanner
	{
	public:
		std::vector<Token> ScanTokens(std::string_view src)
		{
			source = src;
			start = 0;
			current = 0;
			line = 1;
			while (!IsAtEnd())
			{
				start = current;
				ScanToken();
			}

			tokens.emplace_back(TokenType::Eof, "", line);
			std::vector<Token> result;
			std::swap(result, tokens);
			return result;
		}

		void ScanToken()
		{
			char c = Advance();
			switch (c)
			{
			case '(': AddToken(TokenType::LeftParen); break;
			case ')': AddToken(TokenType::RightParen); break;
			case '{': AddToken(TokenType::LeftBrace); break;
			case '}': AddToken(TokenType::RightBrace); break;
			case ',': AddToken(TokenType::Comma); break;
			case '.': AddToken(TokenType::Dot); break;
			case '-': AddToken(TokenType::Minus); break;
			case '+': AddToken(TokenType::Plus); break;
			case ';': AddToken(TokenType::Semicolon); break;
			case '*': AddToken(TokenType::Star); break;
			case '/': AddToken(TokenType::Slash); break;
			case '!': AddToken(Match('=') ? TokenType::BangEqual : TokenType::Bang); break;
			case '=': AddToken(Match('=') ? TokenType::EqualEqual : TokenType::Equal); break;
			case '<': AddToken(Match('=') ? TokenType::LessEqual : TokenType::Less); break;
			case '>': AddToken(Match('=') ? TokenType::GreaterEqual : TokenType::Greater); break;
			case '#':
				while (Peek() != '\n' && !IsAtEnd())
				{
					Advance();
				}
				break;
			case '"':
				while (Peek() != '"' && !IsAtEnd())
				{
					if (Peek() == '\n')
					{
						line++;
					}
					Advance();
				}

				if (IsAtEnd())
				{
					// Error
				}

				Advance();
				AddToken(TokenType::String);
				break;
			case ' ':
			case '\r':
			case '\t':
				break;
			case '\n':
				line++;
				break;
			default:
				if (IsDigit(c))
				{
					while (IsDigit(Peek()))
					{
						Advance();
					}

					if (Peek() == '.' && IsDigit(PeekNext()))
					{
						Advance();
						while (IsDigit(Peek()))
						{
							Advance();
						}
					}

					AddToken(TokenType::Number);
				}
				else if (IsAlpha(c))
				{
					while (IsAlphaNumeric(Peek()))
					{
						Advance();
					}

					auto text = source.substr(start, current - start);
					if (auto search = keywordsMap.find(text); search != keywordsMap.end())
					{
						AddToken(search->second);
						break;
					}


					AddToken(TokenType::Identifier);
				}
				else
				{
					// Error Handling
				}
			}
		}
	private:
		char Advance()
		{
			return source[current++];
		}

		bool Match(char expected)
		{
			if (IsAtEnd()) return false;
			if (source[current] != expected) return false;

			current++;
			return true;
		}

		char Peek()
		{
			return IsAtEnd() ? '\0' : source[current];
		}

		char PeekNext()
		{
			if (current + 1 > source.length()) return '\0';
			return source[current + 1];
		}

		bool IsDigit(char c)
		{
			return c >= '0' && c <= '9';
		}

		bool IsAlpha(char c)
		{
			return (c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| c == '_';
		}

		bool IsAlphaNumeric(char c)
		{
			return IsAlpha(c) || IsDigit(c);
		}

		void AddToken(TokenType type)
		{
			tokens.emplace_back(type, source.substr(start, current - start), line);
		}

		bool IsAtEnd()
		{
			return current >= source.length();
		}

		int start;
		int current;
		int line;
		std::vector<Token> tokens;
		std::string_view source;
	};
}
