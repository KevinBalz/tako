#pragma once
#include "AST.hpp"
#include <string>
#include "Utility.hpp"
#include <memory>

namespace tako::Scripting
{
	enum class ScriptType
	{
		Number,
		Bool,
		String,
		Nil
	};

	struct ScriptValue
	{
		union
		{
			float number;
			bool boolean;
			std::string str;
		};
		ScriptType type;

		ScriptValue()
		{
			number = 0;
			type = ScriptType::Nil;
		}

		ScriptValue(bool b)
		{
			boolean = b;
			type = ScriptType::Bool;
		}

		ScriptValue(float n)
		{
			number = n;
			type = ScriptType::Number;
		}

		ScriptValue(std::string s)
		{
			new (&str) std::string(std::move(s));
			type = ScriptType::String;
		}

		~ScriptValue()
		{
			switch (type)
			{
				case ScriptType::String:
					std::destroy_at(&str);
					break;
			}
		}

		bool IsTruthy()
		{
			if (type == ScriptType::Nil) return false;
			if (type == ScriptType::Bool) return boolean;
			return true;
		}

		bool operator==(const ScriptValue& other)
		{
			if (type != other.type) return false;
			switch (type)
			{
				case ScriptType::Nil: return true;
				case ScriptType::Number: return number == other.number;
				case ScriptType::Bool: return boolean == other.boolean;
				case ScriptType::String: return str == other.str;
			}
		}

		void Print()
		{
			switch (type)
			{
				case ScriptType::Nil:
					LOG("nil");
					break;
				case ScriptType::Number:
					LOG("{}", number);
					break;
				case ScriptType::Bool:
					LOG("{}", boolean);
					break;
				case ScriptType::String:
					LOG("{}", str);
					break;
			}
		}
	};

	ScriptValue operator+(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
			case ScriptType::Number:
				return ScriptValue(a.number + b.number);
			case ScriptType::String:
				return ScriptValue(a.str + b.str);
		}
	}

	ScriptValue operator-(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
			case ScriptType::Number:
				return ScriptValue(a.number - b.number);
		}
	}

	ScriptValue operator*(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
		case ScriptType::Number:
			return ScriptValue(a.number * b.number);
		}
	}

	ScriptValue operator/(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
		case ScriptType::Number:
			return ScriptValue(a.number / b.number);
		}
	}

	class Interpreter
	{
	public:
		void Interpret(const Program& prog)
		{
			for (auto& stmt : prog.statements)
			{
				Evaluate(stmt);
			}
		}

		void Evaluate(const Statement& stmt)
		{
			std::visit([this](auto& st)
			{
				using T = std::decay_t<decltype(st)>;
				if constexpr (std::is_same_v <T, ExpressionStatement>)
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
	};
}
