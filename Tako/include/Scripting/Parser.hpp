#pragma once
#include "AST.hpp"
#include "Scanner.hpp"
#include <vector>

namespace tako::Scripting
{
	class Parser
	{
	public:
		Parser(std::vector<Token> tokens)
		{
			this->tokens = std::move(tokens);
		}

		Program Parse()
		{
			Program prog;
			while (!IsAtEnd())
			{
				prog.declarations.emplace_back(ParseDeclaration());
			}

			return prog;
		}

	private:
		Declaration ParseDeclaration()
		{
			if (Match(TokenType::Var))
			{
				return ParseVariableDeclaration();
			}

			return ParseStatement();
		}

		Declaration ParseVariableDeclaration()
		{
			VariableDeclaration dec;
			dec.identifier = Consume(TokenType::Identifier, "Expected variable name").lexeme;
			if (Match(TokenType::Equal))
			{
				dec.initializer = ParseExpression();
			}

			Consume(TokenType::Semicolon, "Expected ';' after variable declaration");
			return dec;
		}

		Statement ParseStatement()
		{
			if (Match(TokenType::Print)) return ParsePrintStatement();

			return ParseExpressionStatement();
		}

		Statement ParsePrintStatement()
		{
			PrintStatement stmt;
			stmt.expr = ParseExpression();
			Consume(TokenType::Semicolon, "Expected ';' after an expression");
			return stmt;
		}

		Statement ParseExpressionStatement()
		{
			ExpressionStatement stmt;
			stmt.expr = ParseExpression();
			Consume(TokenType::Semicolon, "Expected ';' after an expression");
			return stmt;
		}

		Expression ParseExpression()
		{
			return ParseAssignment();
		}

		Expression ParseAssignment()
		{
			Expression expr = ParseEquality();

			if (Match(TokenType::Equal))
			{
				auto value = std::make_unique<Expression>(ParseAssignment());

				if (std::holds_alternative<VariableAccess>(expr))
				{
					Assign ass;
					ass.name = std::get<VariableAccess>(expr).identifier;
					ass.value = std::move(value);
					return ass;
				}
			}

			return expr;
		}

		Expression ParseEquality()
		{
			Expression expr = ParseComparison();

			while (Match(TokenType::BangEqual, TokenType::EqualEqual))
			{
				Token op = Previous();
				auto right = std::make_unique<Expression>(ParseComparison());
				expr = BinaryExpression(std::make_unique<Expression>(std::move(expr)), op.type, std::move(right));
			}

			return expr;
		}

		Expression ParseComparison()
		{
			Expression expr = ParseTerm();

			while (Match(TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual))
			{
				Token op = Previous();
				auto right = std::make_unique<Expression>(ParseTerm());
				expr = BinaryExpression(std::make_unique<Expression>(std::move(expr)), op.type, std::move(right));
			}

			return expr;
		}

		Expression ParseTerm()
		{
			Expression expr = ParseFactor();

			while (Match(TokenType::Minus, TokenType::Plus))
			{
				Token op = Previous();
				auto right = std::make_unique<Expression>(ParseFactor());
				expr = BinaryExpression(std::make_unique<Expression>(std::move(expr)), op.type, std::move(right));
			}

			return expr;
		}

		Expression ParseFactor()
		{
			Expression expr = ParseUnary();

			while (Match(TokenType::Slash, TokenType::Star))
			{
				Token op = Previous();
				auto right = std::make_unique<Expression>(ParseUnary());
				expr = BinaryExpression(std::make_unique<Expression>(std::move(expr)), op.type, std::move(right));
			}

			return expr;
		}

		Expression ParseUnary()
		{
			if (Match(TokenType::Bang, TokenType::Minus))
			{
				Token op = Previous();
				auto right = std::make_unique<Expression>(ParseUnary());
				return UnaryExpression(op.type, std::move(right));
			}

			return ParsePrimary();
		}

		Expression ParsePrimary()
		{
			if (Match(TokenType::False)) return BoolLiteral{ false };
			if (Match(TokenType::True)) return BoolLiteral{ true };
			if (Match(TokenType::Nil)) return NilLiteral();

			if (Match(TokenType::Number))
			{
				float number = std::stof(std::string{ Previous().lexeme });
				return NumberLiteral{ number };
			}

			if (Match(TokenType::String))
			{
				auto lexeme = Previous().lexeme;
				return StringLiteral{ lexeme.substr(1, lexeme.length() - 2) };
			}

			if (Match(TokenType::Identifier))
			{
				VariableAccess acc;
				acc.identifier = Previous().lexeme;
				return acc;
			}

			if (Match(TokenType::LeftParen))
			{
				Expression expr = ParseExpression();
				Consume(TokenType::RightParen, "Expecteded closing ')'");
				return expr;
			}

			//LOG_ERR("Expected expression");
		}

		template<typename... Args>
		bool Match(TokenType type, Args... args)
		{
			if (Check(type))
			{
				Advance();
				return true;
			}
			return Match(args...);
		}

		bool Match()
		{
			return false;
		}

		bool Check(TokenType type)
		{
			if (IsAtEnd()) return false;
			return Peek().type == type;
		}

		Token Advance()
		{
			if (!IsAtEnd())
			{
				current++;
			}
			return Previous();
		}

		Token Consume(TokenType type, std::string_view message)
		{
			if (Check(type)) return Advance();
			//LOG_ERR("{}", message); //TODO: Error
		}

		bool IsAtEnd()
		{
			return Peek().type == TokenType::Eof;
		}

		Token Peek()
		{
			return tokens[current];
		}

		Token Previous()
		{
			return tokens[current - 1];
		}

		int current = 0;
		std::vector<Token> tokens;
	};
}
