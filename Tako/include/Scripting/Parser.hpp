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

	//private:
		Declaration ParseDeclaration()
		{
			if (Match(TokenType::Class)) return ParseClass();
			//if (Match(TokenType::Fun)) return ParseFunction();
			if (Match(TokenType::Var)) return ParseVariableDeclaration();

			return ParseStatement();
		}

		Declaration ParseClass()
		{
			ClassDeclaration dec;
			dec.name = Consume(TokenType::Identifier, "Expect class name").lexeme;
			Consume(TokenType::LeftBrace, "Expect '{' before class body");

			while (!Check(TokenType::RightBrace) && !IsAtEnd())
			{
				//dec.methods.emplace_back(ParseFunction());
			}

			Consume(TokenType::RightBrace, "Expect '}' before class body.");

			return dec;
		}

		/*
		Declaration ParseFunction()
		{
			FunctionDeclaration dec;
			dec.name = Consume(TokenType::Identifier, "Expect function name").lexeme;
			Consume(TokenType::LeftParen, "Expect '(' after function name");
			if (!Check(TokenType::RightParen))
			{
				do
				{
					dec.params.emplace_back(Consume(TokenType::Identifier, "Expect parameter name").lexeme);
				}
				while (Match(TokenType::Comma));
			}
			Consume(TokenType::RightParen, "Expect ')' after parameters");

			Consume(TokenType::LeftBrace, "Expect '{' before function body");
			ParseBlockStatements(dec.body);
			return dec;
		}
		*/

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
			if (Match(TokenType::If)) return ParseIfStatement();
			if (Match(TokenType::Print)) return ParsePrintStatement();
			if (Match(TokenType::Return)) return ParseReturnStatement();
			if (Match(TokenType::While)) return ParseWhileStatement();
			if (Match(TokenType::LeftBrace)) return ParseBlock();

			return ParseExpressionStatement();
		}

		Statement ParseIfStatement()
		{
			IfStatement stmt;
			Consume(TokenType::LeftParen, "Expected '(' after 'if'.");
			stmt.condition = ParseExpression();
			Consume(TokenType::RightParen, "Expected ')' after 'if' condition.");

			stmt.then = std::make_unique<Statement>(ParseStatement());
			if (Match(TokenType::Else))
			{
				stmt.elseBranch = std::make_unique<Statement>(ParseStatement());
			}

			return stmt;
		}

		Statement ParseReturnStatement()
		{
			ReturnStatement stmt;
			if (!Check(TokenType::Semicolon))
			{
				stmt.value = ParseExpression();
			}

			Consume(TokenType::Semicolon, "Expect ';' after return value");
			return stmt;
		}

		Statement ParseWhileStatement()
		{
			WhileStatement stmt;
			Consume(TokenType::LeftParen, "Expected '(' after 'while'");
			stmt.condition = ParseExpression();
			Consume(TokenType::RightParen, "Expected ')' after 'while' condition");
			stmt.body = std::make_unique<Statement>(ParseStatement());

			return stmt;
		}

		Statement ParsePrintStatement()
		{
			PrintStatement stmt;
			stmt.expr = ParseExpression();
			Consume(TokenType::Semicolon, "Expected ';' after an expression");
			return stmt;
		}

		Statement ParseBlock()
		{
			BlockStatement dec;
			ParseBlockStatements(dec.statements);
			return dec;
		}

		void ParseBlockStatements(std::vector<Declaration>& statements)
		{
			while (!Check(TokenType::RightBrace) && !IsAtEnd())
			{
				statements.emplace_back(ParseDeclaration());
			}

			Consume(TokenType::RightBrace, "Expected '}' after block.");
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
			Expression expr = ParseOr();

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
				else if (std::holds_alternative<Get>(expr))
				{
					Get& get = std::get<Get>(expr);
					Set set;
					set.object = std::move(get.object);
					set.name = get.name;
					set.value = std::move(value);
					return set;
				}
			}

			return expr;
		}

		Expression ParseOr()
		{
			Expression expr = ParseAnd();

			while (Match(TokenType::Or))
			{
				Logical log;
				log.left = std::make_unique<Expression>(std::move(expr));
				log.op = Previous().type;
				log.right = std::make_unique<Expression>(ParseAnd());
				expr = std::move(log);
			}

			return expr;
		}

		Expression ParseAnd()
		{
			Expression expr = ParseEquality();

			while (Match(TokenType::And))
			{
				Logical log;
				log.left = std::make_unique<Expression>(std::move(expr));
				log.op = Previous().type;
				log.right = std::make_unique<Expression>(ParseEquality());
				expr = std::move(log);
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

			return ParseCall();
		}

		Expression ParseCall()
		{
			Expression expr = ParsePrimary();

			while (true)
			{
				if (Match(TokenType::LeftParen))
				{
					Call call;
					call.callee = std::make_unique<Expression>(std::move(expr));
					if (!Check(TokenType::RightParen))
					{
						do
						{
							call.arguments.emplace_back(ParseExpression());
						}
						while (Match(TokenType::Comma));
					}
					Consume(TokenType::RightParen, "Expected ')' after arguments");
					expr = std::move(call);
				}
				else if (Match(TokenType::Dot))
				{
					Get get;
					get.object = std::make_unique<Expression>(std::move(expr));
					get.name = Consume(TokenType::Identifier, "Expect property name after '.'.").lexeme;
					expr = std::move(get);
				}
				else
				{
					break;
				}
			}

			return expr;
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
