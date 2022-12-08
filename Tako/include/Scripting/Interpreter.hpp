#pragma once
#include "AST.hpp"
#include "ScriptValue.hpp"
#include "Environment.hpp"
#include <string>
#include "Utility.hpp"
#include <memory>

namespace tako::Scripting
{
	class Interpreter
	{
	public:
		void Interpret(const Program& prog)
		{
			for (auto& dec : prog.declarations)
			{
				Evaluate(dec);
			}
		}

		void Evaluate(const Declaration& declaration)
		{
			std::visit([this](auto& dec)
			{
				using T = std::decay_t<decltype(dec)>;
				if constexpr (std::is_same_v<T, VariableDeclaration>)
				{
					ScriptValue value;
					if (dec.initializer)
					{
						value = Evaluate(dec.initializer.value());
					}

					environment.Define(std::string(dec.identifier), value);
				}
				else if constexpr (std::is_same_v<T, Statement>)
				{
					Evaluate(dec);
				}
			}, declaration);
		}

		void Evaluate(const Statement& stmt)
		{
			std::visit([this](auto& st)
			{
				using T = std::decay_t<decltype(st)>;
				if constexpr (std::is_same_v<T, ExpressionStatement>)
				{
					Evaluate(st.expr);
				}
				else if constexpr (std::is_same_v<T, PrintStatement>)
				{
					ScriptValue val = Evaluate(st.expr);
					val.Print();
				}
			}, stmt);
		}

		ScriptValue Evaluate(const Expression& expr)
		{
			return std::visit([this](auto& ex) -> ScriptValue
			{
				using T = std::decay_t<decltype(ex)>;
				if constexpr (std::is_same_v<T, Literal>)
				{
					return Evaluate(ex);
				}
				else if constexpr (std::is_same_v<T, UnaryExpression>)
				{
					auto right = Evaluate(*ex.right);
					switch (ex.op)
					{
						case TokenType::Minus:
							return -right.number;
						case TokenType::Bang:
							return !right.IsTruthy();
					}
					return ScriptValue();
				}
				else if constexpr (std::is_same_v<T, BinaryExpression>)
				{
					auto left = Evaluate(*ex.left);
					auto right = Evaluate(*ex.right);
					switch (ex.op)
					{
						case TokenType::Plus:
							return left + right;
						case TokenType::Minus:
							return left - right;
						case TokenType::Star:
							return left * right;
						case TokenType::Slash:
							return left / right;
						case TokenType::Greater:
							return left.number > right.number;
						case TokenType::GreaterEqual:
							return left.number >= right.number;
						case TokenType::Less:
							return left.number < right.number;
						case TokenType::LessEqual:
							return left.number <= right.number;
						case TokenType::EqualEqual:
							return left == right;
						case TokenType::BangEqual:
							return !(left == right);
					}
					return ScriptValue();
				}
				else if constexpr (std::is_same_v<T, Assign>)
				{
					ScriptValue value = Evaluate(*ex.value);
					environment.Assign(std::string(ex.name), value);
					return value;
				}
				else if constexpr (std::is_same_v<T, VariableAccess>)
				{
					return environment.Get(std::string(ex.identifier));
				}
				ASSERT(false);
			}, expr);
		}

		ScriptValue Evaluate(const Literal& literal)
		{
			return std::visit([this](auto& lt) -> ScriptValue
			{
				using T = std::decay_t<decltype(lt)>;
				if constexpr (std::is_same_v<T, NilLiteral>)
				{
					return ScriptValue();
				}
				else if constexpr (std::is_same_v<T, NumberLiteral>)
				{
					return ScriptValue(lt.value);
				}
				else if constexpr (std::is_same_v<T, BoolLiteral>)
				{
					return ScriptValue(lt.value);
				}
				else if constexpr (std::is_same_v<T, StringLiteral>)
				{
					return ScriptValue(std::string(lt.value));
				}
				ASSERT(false);
			}, literal);
		}

		Environment environment;
	};
}
