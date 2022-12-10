#pragma once
#include <variant>
#include <optional>
#include "Scanner.hpp"

namespace tako::Scripting
{
	struct NilLiteral
	{
	};

	struct NumberLiteral
	{
		float value;
	};

	struct BoolLiteral
	{
		bool value;
	};

	struct StringLiteral
	{
		std::string_view value;
	};

	using Literal = std::variant<NilLiteral, NumberLiteral, BoolLiteral, StringLiteral>;

	struct VariableAccess
	{
		std::string_view identifier;
	};

	struct UnaryExpression;
	struct BinaryExpression;
	struct Assign;
	struct Logical;
	using Expression = std::variant<Literal, UnaryExpression, BinaryExpression, Assign, VariableAccess, Logical>;

	struct UnaryExpression
	{
		UnaryExpression() {}
		UnaryExpression(TokenType op, std::unique_ptr<Expression>&& right) : op(op), right(std::move(right)) {}
		TokenType op;
		std::unique_ptr<Expression> right;
	};

	struct BinaryExpression
	{
		BinaryExpression() {}
		BinaryExpression(std::unique_ptr<Expression>&& left, TokenType op, std::unique_ptr<Expression>&& right) : left(std::move(left)), op(op), right(std::move(right)) {}

		std::unique_ptr<Expression> left;
		TokenType op;
		std::unique_ptr<Expression> right;
	};

	struct Assign
	{
		std::string_view name;
		std::unique_ptr<Expression> value;
	};

	struct Logical
	{
		std::unique_ptr<Expression> left;
		TokenType op;
		std::unique_ptr<Expression> right;
	};

	struct ExpressionStatement;
	struct PrintStatement;
	struct BlockStatement;
	struct IfStatement;
	struct WhileStatement;
	using Statement = std::variant<ExpressionStatement, PrintStatement, BlockStatement, IfStatement, WhileStatement>;

	struct ExpressionStatement
	{
		Expression expr;
	};

	struct PrintStatement
	{
		Expression expr;
	};

	struct VariableDeclaration
	{
		std::string_view identifier;
		std::optional<Expression> initializer;
	};

	struct IfStatement
	{
		Expression condition;
		std::unique_ptr<Statement> then;
		std::optional<std::unique_ptr<Statement>> elseBranch;
	};

	struct WhileStatement
	{
		Expression condition;
		std::unique_ptr<Statement> body;
	};

	using Declaration = std::variant<VariableDeclaration, Statement>;

	struct BlockStatement
	{
		std::vector<Declaration> statements;
	};

	struct Program
	{
		std::vector<Declaration> declarations;
	};
}
