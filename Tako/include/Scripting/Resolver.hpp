#pragma once
#include "AST.hpp"
#include <stack>
#include <unordered_map>
#include <string_view>

namespace tako::Scripting
{
	class Resolver
	{
	public:
		void Resolve(const Declaration& declaration)
		{
			std::visit([this](auto& dec)
			{
				using T = std::decay_t<decltype(dec)>;
				if constexpr (std::is_same_v<T, VariableDeclaration>)
				{
					Declare(dec.identifier);
					if (dec.initializer)
					{
						Resolve(dec.initializer.value());
					}
					Define(dec.identifier);
				}
				else if constexpr (std::is_same_v<T, FunctionDeclaration>)
				{
					Declare(dec.name);
					Define(dec.name);

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
					EndScope();
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
				Declare(param);
				Define(param);
			}
			Resolve(function.body);
			EndScope();
		}

		void BeginScope()
		{
			scopes.push_back({});
		}

		void EndScope()
		{
			scopes.pop_back();
		}

		void Declare(std::string_view name)
		{
			if (scopes.empty()) return;

			auto& scope = scopes.back();
			scope[std::string(name)] = false;
		}

		void Define(std::string_view name)
		{
			if (scopes.empty()) return;

			auto& scope = scopes.back();
			scope[std::string(name)] = true;
		}

		void ResolveLocal(const Expression* expr, std::string_view name)
		{
			for (int i = scopes.size() - 1; i >= 0; i--)
			{
				auto& scope = scopes[i];
				if (auto search = scope.find(std::string(name)); search != scope.end())
				{
					// Resolve(expr, scopes.size() - 1 - i);
					locals[expr] = scopes.size() - 1 - i;
					return;
				}
			}
		}

		std::unordered_map<const Expression*, int> locals;
		std::vector<std::unordered_map<std::string, bool>> scopes;
	};
}
