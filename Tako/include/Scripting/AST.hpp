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
	struct Call;
	struct Assign;
	struct Logical;
	using Expression = std::variant<Literal, UnaryExpression, BinaryExpression, Call, Assign, VariableAccess, Logical>;

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

	struct Call
	{
		std::unique_ptr<Expression> callee;
		std::vector<Expression> arguments;
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

	struct ReturnStatement
	{
		std::optional<Expression> value;
	};

	struct ExpressionStatement;
	struct PrintStatement;
	struct BlockStatement;
	struct IfStatement;
	struct WhileStatement;
	using Statement = std::variant<ExpressionStatement, PrintStatement, BlockStatement, IfStatement, WhileStatement, ReturnStatement>;

	struct ExpressionStatement
	{
		Expression expr;
	};

	struct PrintStatement
	{
		Expression expr;
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

	// Declarations

	struct VariableDeclaration
	{
		std::string_view identifier;
		std::optional<Expression> initializer;
	};

	struct FunctionDeclaration;
	using Declaration = std::variant<VariableDeclaration, FunctionDeclaration, Statement>;


	struct FunctionDeclaration
	{
		std::string_view name;
		std::vector<std::string_view> params;
		std::vector<Declaration> body;
	};

	struct BlockStatement
	{
		std::vector<Declaration> statements;
	};

	struct Program
	{
		std::vector<Declaration> declarations;
	};
}
