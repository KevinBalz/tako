#pragma once
#include "Chunk.hpp"

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
			double b = Pop(); \
			double a = Pop(); \
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
						ScriptValue constant = READ_CONSTANT();
						Push(constant);
						break;
					}
					case OpCode::ADD: BINARY_OP(+); break;
					case OpCode::SUBTRACT: BINARY_OP(-); break;
					case OpCode::MULTIPLY: BINARY_OP(*); break;
					case OpCode::DIVIDE: BINARY_OP(/); break;
					case OpCode::NEGATE: Push(-Pop()); break;
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

		void Push(ScriptValue value)
		{
			*stackTop = value;
			stackTop++;
		}

		ScriptValue Pop()
		{
			stackTop--;
			return *stackTop;
		}

		void ResetStack()
		{
			stackTop = stack;
		}

		Chunk* chunk;
		U8* ip;
		ScriptValue stack[STACK_MAX];
		ScriptValue* stackTop;
	};
}
