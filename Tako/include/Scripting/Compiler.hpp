#pragma once
#include "AST.hpp"
#include "Chunk.hpp"

namespace tako::Scripting
{
	class Compiler
	{
	public:

		void Compile(Chunk* chunk, const Program& ast)
		{
			compilingChunk = chunk;
			for (auto& dec : ast.declarations)
			{
				Compile(dec);
			}
			Emit(OpCode::RETURN);
		}

		void Compile(const Declaration& declaration)
		{
			std::visit([this](auto& dec)
			{
				using T = std::decay_t<decltype(dec)>;
				if constexpr (std::is_same_v<T, VariableDeclaration>)
				{
					auto global = IdentifierConstant(dec.identifier);
					if (dec.initializer)
					{
						Compile(dec.initializer.value());
					}
					else
					{
						Emit(OpCode::NIL);
					}

					DefineVariable(global);
				}
				else if constexpr (std::is_same_v<T, FunctionDeclaration>)
				{
				}
				else if constexpr (std::is_same_v<T, Statement>)
				{
					Compile(dec);
				}
				else
				{
					ASSERT(false);
				}
			}, declaration);
		}

		void Compile(const Statement& statement)
		{
			return std::visit([this](auto& stmt)
			{
				using T = std::decay_t<decltype(stmt)>;
				if constexpr (std::is_same_v<T, ExpressionStatement>)
				{
					Compile(stmt.expr);
					Emit(OpCode::POP);
				}
				else if constexpr (std::is_same_v<T, PrintStatement>)
				{
					Compile(stmt.expr);
					Emit(OpCode::PRINT);
				}
				else if constexpr (std::is_same_v<T, BlockStatement>)
				{
				}
				else if constexpr (std::is_same_v<T, IfStatement>)
				{
				}
				else if constexpr (std::is_same_v<T, WhileStatement>)
				{
				}
				else if constexpr (std::is_same_v<T, ReturnStatement>)
				{
				}
				else
				{
					ASSERT(false);
				}
			}, statement);
		}

		void Compile(const Expression& expression)
		{
			std::visit([this](auto& expr)
			{
				using T = std::decay_t<decltype(expr)>;
				if constexpr (std::is_same_v<T, Literal>)
				{
					Compile(expr);
				}
				else if constexpr (std::is_same_v<T, UnaryExpression>)
				{
					Compile(*expr.right);
					switch (expr.op)
					{
						case TokenType::Bang: Emit(OpCode::NOT); break;
						case TokenType::Minus: Emit(OpCode::NEGATE); break;
						default: ASSERT(false);
					}
				}
				else if constexpr (std::is_same_v<T, BinaryExpression>)
				{
					Compile(*expr.left);
					Compile(*expr.right);

					switch (expr.op)
					{
						case TokenType::BangEqual: Emit(OpCode::EQUAL, OpCode::NOT); break;
						case TokenType::EqualEqual: Emit(OpCode::EQUAL); break;
						case TokenType::Greater: Emit(OpCode::GREATER); break;
						case TokenType::GreaterEqual: Emit(OpCode::LESS, OpCode::NOT); break;
						case TokenType::Less: Emit(OpCode::LESS); break;
						case TokenType::LessEqual: Emit(OpCode::GREATER, OpCode::NOT); break;
						case TokenType::Plus: Emit(OpCode::ADD); break;
						case TokenType::Minus: Emit(OpCode::SUBTRACT); break;
						case TokenType::Star: Emit(OpCode::MULTIPLY); break;
						case TokenType::Slash: Emit(OpCode::DIVIDE); break;
					}
				}
				else if constexpr (std::is_same_v<T, Call>)
				{
				}
				else if constexpr (std::is_same_v<T, Assign>)
				{
					auto arg = IdentifierConstant(expr.name);
					Compile(*expr.value);
					Emit(OpCode::SET_GLOBAL, arg);
				}
				else if constexpr (std::is_same_v<T, VariableAccess>)
				{
					auto arg = IdentifierConstant(expr.identifier);
					Emit(OpCode::GET_GLOBAL, arg);
				}
				else if constexpr (std::is_same_v<T, Logical>)
				{
				}
			}, expression);
		}

		void Compile(const Literal& literal)
		{
			std::visit([this](auto& lt)
			{
				using T = std::decay_t<decltype(lt)>;
				if constexpr (std::is_same_v<T, NilLiteral>)
				{
					Emit(OpCode::NIL);
				}
				else if constexpr (std::is_same_v<T, NumberLiteral>)
				{
					EmitConstant(lt.value);
				}
				else if constexpr (std::is_same_v<T, BoolLiteral>)
				{
					Emit(lt.value ? OpCode::TRUE : OpCode::FALSE);
				}
				else if constexpr (std::is_same_v<T, StringLiteral>)
				{
					EmitConstant(new ObjString(lt.value));
				}
				else
				{
					ASSERT(false); //unreacheable
				}
			}, literal);
		}



		void Emit(U8 byte)
		{
			currentChunk()->Write(byte, 123);
		}

		void Emit(OpCode op)
		{
			Emit((U8) op);
		}

		void Emit(U8 byte1, U8 byte2)
		{
			Emit(byte1);
			Emit(byte2);
		}

		void Emit(OpCode code, U8 byte)
		{
			Emit((U8)code, byte);
		}

		void Emit(OpCode code1, OpCode code2)
		{
			Emit((U8) code1, (U8) code2);
		}

		void EmitConstant(DynamicValue value)
		{
			Emit(OpCode::CONSTANT, currentChunk()->AddConstant(value));
		}

		U8 IdentifierConstant(std::string_view identifier)
		{
			return currentChunk()->AddConstant(new ObjString(identifier));
		}

		void DefineVariable(U8 global)
		{
			Emit(OpCode::DEFINE_GLOBAL, global);
		}

	private:

		Chunk* currentChunk()
		{
			return compilingChunk;
		}

		Chunk* compilingChunk;
	};
}
