#pragma once
#include "AST.hpp"
#include "Chunk.hpp"
#include "Resolver.hpp"
#include "Object.hpp"

namespace tako::Scripting
{
	enum class FunctionType
	{
		Function,
		Script
	};

	class Compiler
	{
	public:

		ObjFunction* Compile(Resolver* resolver, const Program& ast)
		{
			this->resolver = resolver;
			this->function = nullptr;
			this->function = new ObjFunction();
			for (auto& dec : ast.declarations)
			{
				Compile(dec);
			}
			Emit(OpCode::RETURN);
			return this->function;
		}

		void Compile(const Declaration& declaration)
		{
			std::visit([this](auto& dec)
			{
				using T = std::decay_t<decltype(dec)>;
				if constexpr (std::is_same_v<T, VariableDeclaration>)
				{
					if (dec.initializer)
					{
						Compile(dec.initializer.value());
					}
					else
					{
						Emit(OpCode::NIL);
					}

					if (resolver->globalDeclarations[&dec])
					{
						auto global = IdentifierConstant(dec.identifier);
						DefineVariable(global);
					}
					// if its not global, its a local variable on the stack so nothing to do here
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
					for (auto& statement : stmt.statements)
					{
						Compile(statement);
					}
					int popCount = resolver->popCounts[&stmt];
					for (int i = 0; i < popCount; i++)
					{
						Emit(OpCode::POP);
					}
				}
				else if constexpr (std::is_same_v<T, IfStatement>)
				{
					Compile(stmt.condition);
					auto thenJump = EmitJump(OpCode::JUMP_IF_FALSE);
					Emit(OpCode::POP);

					Compile(*stmt.then);

					auto elseJump = EmitJump(OpCode::JUMP);
					PatchJump(thenJump);
					Emit(OpCode::POP);

					if (stmt.elseBranch)
					{
						Compile(*stmt.elseBranch.value());
					}
					PatchJump(elseJump);
				}
				else if constexpr (std::is_same_v<T, WhileStatement>)
				{
					int loopStart = currentChunk()->code.size();
					Compile(stmt.condition);
					int exitJump = EmitJump(OpCode::JUMP_IF_FALSE);

					Emit(OpCode::POP);
					Compile(*stmt.body);
					EmitLoop(loopStart);

					PatchJump(exitJump);
					Emit(OpCode::POP);
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
			std::visit([this, &expression](auto& expr)
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
					Compile(*expr.value);

					int stackPos = resolver->stackPositions[&expression];
					if (stackPos < 0)
					{
						auto arg = IdentifierConstant(expr.name);
						Emit(OpCode::SET_GLOBAL, arg);
					}
					else
					{
						Emit(OpCode::SET_LOCAL, stackPos);
					}
				}
				else if constexpr (std::is_same_v<T, VariableAccess>)
				{
					int stackPos = resolver->stackPositions[&expression];
					if (stackPos < 0)
					{
						auto arg = IdentifierConstant(expr.identifier);
						Emit(OpCode::GET_GLOBAL, arg);
					}
					else
					{
						Emit(OpCode::GET_LOCAL, stackPos);
					}

				}
				else if constexpr (std::is_same_v<T, Logical>)
				{
					Compile(*expr.left);

					auto endJump = EmitJump(expr.op == TokenType::And ? OpCode::JUMP_IF_FALSE : OpCode::JUMP_IF_TRUE);

					Emit(OpCode::POP);
					Compile(*expr.right);

					PatchJump(endJump);
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
			// if scope > 0 return;
			Emit(OpCode::DEFINE_GLOBAL, global);
		}

		int EmitJump(OpCode instruction)
		{
			Emit(instruction);
			Emit(0xff);
			Emit(0xff);
			return currentChunk()->code.size() - 2;
		}

		void PatchJump(int offset)
		{
			int jump = currentChunk()->code.size() - offset - 2;

			if (jump > std::numeric_limits<U16>::max())
			{
				LOG_ERR("Too much code to jump over.");
			}

			currentChunk()->code[offset] = (jump >> 8) & 0xff;
			currentChunk()->code[offset + 1] = jump & 0xff;
		}

		void EmitLoop(int loopStart)
		{
			Emit(OpCode::LOOP);

			int offset = currentChunk()->code.size() - loopStart + 2;
			if (offset > std::numeric_limits<U16>::max())
			{
				LOG_ERR("Loop body to large.");
			}

			Emit((offset >> 8) & 0xff);
			Emit(offset & 0xff);
		}

	private:

		Chunk* currentChunk()
		{
			return &function->chunk;
		}

		ObjFunction* function;
		FunctionType type;
		Resolver* resolver;
	};
}
