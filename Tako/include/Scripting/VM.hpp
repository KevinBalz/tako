#pragma once
#include "Chunk.hpp"
#include "Parser.hpp"
#include "Compiler.hpp"

//#define DEBUG_TRACE_EXECUTION

#define STACK_MAX 256

namespace tako::Scripting
{
	enum class InterpretResult
	{
		OK,
		COMPILE_ERROR,
		RUNTIME_ERROR
	};

	class VM
	{
	public:

		void Init()
		{
			ResetStack();
		}

		InterpretResult Interpret(std::string_view source)
		{
			Parser parser(Scanner().ScanTokens(source));
			Chunk chunk;

			Compiler comp;
			comp.Compile(&chunk, parser.ParseExpression());

			chunk.Disassemble("test");

			return Interpret(&chunk);
		}

		InterpretResult Interpret(Chunk* chunk)
		{
			this->chunk = chunk;
			ip = &chunk->code[0];
			return Run();
		}

		InterpretResult Run()
		{
#define READ_BYTE() (*ip++)
#define READ_OPCODE() ((OpCode) *ip++)
#define READ_CONSTANT() (chunk->constants[READ_BYTE()])
#define BINARY_OP(op) \
		do \
		{ \
			if (!Peek(0).IsNumber() || !Peek(1).IsNumber()) \
			{ \
				LOG_ERR("Operand must be a number."); \
				return InterpretResult::RUNTIME_ERROR; \
			} \
			double b = Pop().as.number; \
			double a = Pop().as.number; \
			Push(a op b); \
		} while (false)

			for (;;)
			{
#ifdef DEBUG_TRACE_EXECUTION
				// Trace stack
				chunk->DisassembleInstruction((int)(ip - &chunk->code[0]));
#endif
				switch (auto intruction = READ_OPCODE())
				{
					case OpCode::CONSTANT:
					{
						DynamicValue constant = READ_CONSTANT();
						Push(constant);
						break;
					}
					case OpCode::NIL: Push(DynamicValue()); break;
					case OpCode::TRUE: Push(true); break;
					case OpCode::FALSE: Push(false); break;
					case OpCode::ADD: BINARY_OP(+); break;
					case OpCode::SUBTRACT: BINARY_OP(-); break;
					case OpCode::MULTIPLY: BINARY_OP(*); break;
					case OpCode::DIVIDE: BINARY_OP(/); break;
					case OpCode::NOT:
						Push(!Pop().IsTruthy());
						break;
					case OpCode::NEGATE:
					{
						if (!Peek(0).IsNumber())
						{
							LOG_ERR("Operand must be a number.");
							return InterpretResult::RUNTIME_ERROR;
						}
						Push(-Pop().as.number);
						break;
					}
					case OpCode::RETURN:
					{
						PrintValue(Pop());
						std::cout << "\n";
						return InterpretResult::OK;
					}
				}
			}
#undef READ_BYTE
#undef READ_OPCODE
#undef READ_CONSTANT
#undef BINARY_OP
		}

		void Push(DynamicValue value)
		{
			*stackTop = value;
			stackTop++;
		}

		DynamicValue Pop()
		{
			stackTop--;
			return *stackTop;
		}

		DynamicValue Peek(int distance)
		{
			return stackTop[-1 - distance];
		}

		void ResetStack()
		{
			stackTop = stack;
		}

		Chunk* chunk;
		U8* ip;
		DynamicValue stack[STACK_MAX];
		DynamicValue* stackTop;
	};
}
