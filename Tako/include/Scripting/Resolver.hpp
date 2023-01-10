#pragma once
#include "AST.hpp"
#include <stack>
#include <unordered_map>
#include <string_view>

namespace tako::Scripting
{
	struct Local
	{
		std::string_view name;
		int depth;
	};

	class Resolver
	{
	public:
		void Resolve(const Program& ast)
		{
			locals.push_back({"", 0});
			for (auto& dec : ast.declarations)
			{
				Resolve(dec);
			}
		}

		void Resolve(const Declaration& declaration)
		{
			std::visit([this](auto& dec)
			{
				using T = std::decay_t<decltype(dec)>;
				if constexpr (std::is_same_v<T, VariableDeclaration>)
				{
					if (scopeDepth > 0)
					{
						auto name = dec.identifier;
						for (int i = locals.size() - 1; i>= 0; i--)
						{
							auto& local = locals[i];
							if (local.depth != -1 && local.depth < scopeDepth)
							{
								break;
							}

							if (name == local.name)
							{
								LOG_ERR("Already a variable named '{}' in the same scope", name)
							}
						}
						AddLocal(dec.identifier);
						globalDeclarations[&dec] = false;
					}
					else
					{
						globalDeclarations[&dec] = true;
					}
					if (dec.initializer)
					{
						Resolve(dec.initializer.value());
					}
					if (scopeDepth > 0)
					{
						locals.back().depth = scopeDepth;
					}
				}
				else if constexpr (std::is_same_v<T, FunctionDeclaration>)
				{
					//Declare(dec.name);
					//Define(dec.name);

					ResolveFunction(dec);
				}
				else if constexpr (std::is_same_v<T, Statement>)
				{
					Resolve(dec);
				}
			}, declaration);
		}

		void Resolve(const Statement& statement)
		{
			std::visit([this](auto& stmt)
			{
				using T = std::decay_t<decltype(stmt)>;
				if constexpr (std::is_same_v<T, ExpressionStatement>)
				{
					Resolve(stmt.expr);
				}
				else if constexpr (std::is_same_v<T, PrintStatement>)
				{
					Resolve(stmt.expr);
				}
				else if constexpr (std::is_same_v<T, BlockStatement>)
				{
					BeginScope();
					Resolve(stmt.statements);
					popCounts[&stmt] = EndScope();
				}
				else if constexpr (std::is_same_v<T, IfStatement>)
				{
					Resolve(stmt.condition);
					Resolve(*stmt.then);
					if (stmt.elseBranch)
					{
						Resolve(*stmt.elseBranch.value());
					}
				}
				else if constexpr (std::is_same_v<T, WhileStatement>)
				{
					Resolve(stmt.condition);
					Resolve(*stmt.body);
				}
				else if constexpr (std::is_same_v<T, ReturnStatement>)
				{
					if (stmt.value)
					{
						Resolve(stmt.value.value());
					}
				}
			}, statement);
		}

		void Resolve(const Expression& expression)
		{
			auto exprPtr = &expression;
			std::visit([this, exprPtr](auto& expr)
			{
				using T = std::decay_t<decltype(expr)>;
				if constexpr (std::is_same_v<T, Literal>)
				{
				}
				else if constexpr (std::is_same_v<T, UnaryExpression>)
				{
					Resolve(*expr.right);
				}
				else if constexpr (std::is_same_v<T, BinaryExpression>)
				{
					Resolve(*expr.left);
					Resolve(*expr.right);
				}
				else if constexpr (std::is_same_v<T, Call>)
				{
					Resolve(*expr.callee);
					for (auto& argument : expr.arguments)
					{
						Resolve(argument);
					}
				}
				else if constexpr (std::is_same_v<T, Assign>)
				{
					Resolve(*expr.value);
					ResolveLocal(exprPtr, expr.name);
				}
				else if constexpr (std::is_same_v<T, VariableAccess>)
				{
					//TODO: check for initializing variable with itself

					ResolveLocal(exprPtr, expr.identifier);

				}
				else if constexpr (std::is_same_v<T, Logical>)
				{
					Resolve(*expr.left);
					Resolve(*expr.right);
				}
			}, expression);
		}

		void Resolve(const std::vector<Declaration>& statements)
		{
			for (auto& stmt : statements)
			{
				Resolve(stmt);
			}
		}

		void ResolveFunction(const FunctionDeclaration& function)
		{
			BeginScope();
			for (auto param : function.params)
			{
				//Declare(param);
				//Define(param);
			}
			Resolve(function.body);
			EndScope();
		}

		void BeginScope()
		{
			scopeDepth++;
		}

		int EndScope()
		{
			scopeDepth--;

			int popCount = 0;
			while (!locals.empty() && locals.back().depth > scopeDepth)
			{
				popCount++;
				locals.pop_back();
			}

			return popCount;
		}


		void AddLocal(std::string_view name)
		{
			locals.push_back({name, -1});
		}

		void ResolveLocal(const Expression* expression, std::string_view name)
		{
			for (int i = locals.size() - 1; i >= 0; i--)
			{
				auto& local = locals[i];
				if (local.name == name)
				{
					if (local.depth == -1)
					{
						LOG_ERR("Can't read local variable in its own initializer.");
					}
					stackPositions[expression] = i;
					return;
				}
			}

			stackPositions[expression] = -1;
		}

		int scopeDepth = 0;
		std::vector<Local> locals;
		std::unordered_map<const VariableDeclaration*, bool> globalDeclarations;
		std::unordered_map<const BlockStatement*, int> popCounts;
		std::unordered_map<const Expression*, int> stackPositions;
		//std::vector<std::unordered_map<std::string, bool>> scopes;
	};
}
