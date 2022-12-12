#pragma once
#include "AST.hpp"
#include "ScriptValue.hpp"
#include "Environment.hpp"
#include <string>
#include "Utility.hpp"
#include <memory>

namespace tako::Scripting
{

	ScriptValue PrintFunc(Interpreter* interpreter, std::vector<ScriptValue>& arguments);

	class Interpreter
	{
	public:
		Interpreter()
		{
			globalEnvironment = std::make_shared<Environment>();
			globalEnvironment->Define("print", &PrintFunc);
		}

		void Interpret(const Program& prog)
		{
			ExecuteBlock(prog.declarations, globalEnvironment);
		}

		std::optional<ScriptValue> ExecuteBlock(const std::vector<Declaration>& statements, std::shared_ptr<Environment> env)
		{
			std::optional<ScriptValue> ret;
			auto previous = environment;
			environment = env;

			for (auto& s : statements)
			{
				if (auto r = Evaluate(s); r)
				{
					ret = std::move(r);
					break;
				}
			}
			environment = previous;
			return ret;
		}

		std::optional<ScriptValue> Evaluate(const Declaration& declaration)
		{
			return std::visit([this](auto& dec) -> std::optional<ScriptValue>
			{
				using T = std::decay_t<decltype(dec)>;
				if constexpr (std::is_same_v<T, VariableDeclaration>)
				{
					ScriptValue value;
					if (dec.initializer)
					{
						value = Evaluate(dec.initializer.value());
					}

					environment->Define(std::string(dec.identifier), value);
				}
				else if constexpr (std::is_same_v<T, FunctionDeclaration>)
				{
					environment->Define(dec.name, ScriptValue(&dec, environment));
				}
				else if constexpr (std::is_same_v<T, Statement>)
				{
					return Evaluate(dec);
				}
				return {};
			}, declaration);
		}

		std::optional<ScriptValue> Evaluate(const Statement& stmt) 
		{
			return std::visit([this](auto& st) -> std::optional<ScriptValue>
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
				else if constexpr (std::is_same_v<T, BlockStatement>)
				{
					auto newEnv = std::make_shared<Environment>(environment);
					return ExecuteBlock(st.statements, newEnv);
				}
				else if constexpr (std::is_same_v<T, IfStatement>)
				{
					if (Evaluate(st.condition).IsTruthy())
					{
						return Evaluate(*st.then);
					}
					else if (st.elseBranch)
					{
						return Evaluate(*st.elseBranch.value());
					}
				}
				else if constexpr (std::is_same_v<T, WhileStatement>)
				{
					while (Evaluate(st.condition).IsTruthy())
					{
						if (auto r = Evaluate(*st.body); r)
						{
							return r;
						}
					}
				}
				else if constexpr (std::is_same_v<T, ReturnStatement>)
				{
					if (st.value)
					{
						return Evaluate(st.value.value());
					}

					return ScriptValue();
				}
				return {};
			}, stmt);
		}

		ScriptValue Evaluate(const Expression& expr)
		{
			auto exprPtr = &expr;
			return std::visit([this, exprPtr](auto& ex) -> ScriptValue
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
				else if constexpr (std::is_same_v<T, Call>)
				{
					ScriptValue callee = Evaluate(*ex.callee);

					std::vector<ScriptValue> arguments;
					for (auto& e : ex.arguments)
					{
						arguments.emplace_back(Evaluate(e));
					}

					return callee(this, arguments);
				}
				else if constexpr (std::is_same_v<T, Assign>)
				{
					ScriptValue value = Evaluate(*ex.value);
					if (auto search = locals.find(exprPtr); search != locals.end())
					{
						environment->AssignAt(search->second, ex.name, value);
					}
					else
					{
						globalEnvironment->Assign(ex.name, value);
					}

					return value;
				}
				else if constexpr (std::is_same_v<T, VariableAccess>)
				{
					if (auto search = locals.find(exprPtr); search != locals.end())
					{
						return environment->GetAt(search->second, ex.identifier);
					}
					else
					{
						return globalEnvironment->Get(ex.identifier);
					}
				}
				else if constexpr (std::is_same_v<T, Logical>)
				{
					ScriptValue left = Evaluate(*ex.left);

					if (ex.op == TokenType::Or)
					{
						if (left.IsTruthy()) return left;
					}
					else
					{
						if (!left.IsTruthy()) return left;
					}

					return Evaluate(*ex.right);
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

		std::unordered_map<const Expression*, int> locals;
		std::shared_ptr<Environment> globalEnvironment = nullptr;
		std::shared_ptr<Environment> environment = nullptr;

	};

	std::ostream& operator<<(std::ostream& os, const ScriptValue& val)
	{
		switch (val.type)
		{
			case ScriptType::Nil:
				os << "nil";
				break;
			case ScriptType::Number:
				os << val.number;
				break;
			case ScriptType::Bool:
				os << val.boolean;
				break;
			case ScriptType::String:
				os << val.str;
				break;
			case ScriptType::Function:
				os << "<fn " << val.func.declaration->name << ">";
				break;
			case ScriptType::NativeFunction:
				os << "<fn 0x" << val.nativeFunc << ">";
				break;
		}
		return os;
	}

	ScriptValue PrintFunc(Interpreter* interpreter, std::vector<ScriptValue>& arguments)
	{
		for (auto& arg : arguments)
		{
			std::cout << arg << " ";
		}
		std::cout << "\n";
		return {};
	}

	
	ScriptValue ScriptFunction::operator()(Interpreter* interpreter, std::vector<ScriptValue>& arguments)
	{
		auto environment = std::make_shared<Environment>(closure);
		for (int i = 0; i < declaration->params.size(); i++)
		{
			environment->Define(declaration->params[i], arguments[i]);
		}

		auto ret = interpreter->ExecuteBlock(declaration->body, environment);
		if (ret)
		{
			return ret.value();
		}

		return ScriptValue();
	}
}
