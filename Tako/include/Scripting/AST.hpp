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
	using Expression = std::variant<Literal, UnaryExpression, BinaryExpression, Assign, VariableAccess>;

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

	struct ExpressionStatement;
	struct PrintStatement;
	using Statement = std::variant<ExpressionStatement, PrintStatement>;

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

	using Declaration = std::variant<VariableDeclaration, Statement>;

	struct Program
	{
		std::vector<Declaration> declarations;
	};
}
