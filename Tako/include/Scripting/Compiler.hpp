#pragma once
#include "AST.hpp"
#include "Chunk.hpp"

namespace tako::Scripting
{
	class Compiler
	{
	public:

		void Compile(Chunk* chunk, const Expression& ast)
		{
			compilingChunk = chunk;
			Compile(ast);
			Emit(OpCode::RETURN);
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
				}
				else if constexpr (std::is_same_v<T, VariableAccess>)
				{

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

		void EmitConstant(DynamicValue value)
		{
			Emit(OpCode::CONSTANT, currentChunk()->AddConstant(value));
		}

	private:

		Chunk* currentChunk()
		{
			return compilingChunk;
		}

		Chunk* compilingChunk;
	};
}
