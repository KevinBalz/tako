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
			objects = nullptr;
		}

		InterpretResult Interpret(std::string_view source)
		{
			Parser parser(Scanner().ScanTokens(source));
			auto ast = parser.Parse();
			Resolver resolver;
			resolver.Resolve(ast);
			Chunk chunk;

			Compiler comp;
			comp.Compile(&chunk, &resolver, ast);

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
#define READ_STRING() static_cast<ObjString*>(READ_CONSTANT().as.obj)
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
				switch (READ_OPCODE())
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
					case OpCode::POP: Pop(); break;
					case OpCode::GET_LOCAL:
					{
						U8 slot = READ_BYTE();
						Push(stack[slot]);
						break;
					}
					case OpCode::SET_LOCAL:
					{
						U8 slot = READ_BYTE();
						stack[slot] = Peek(0);
						break;
					}
					case OpCode::GET_GLOBAL:
					{
						ObjString* name = READ_STRING();
						if (auto search = globals.find(std::string(name->chars)); search != globals.end())
						{
							Push(search->second);
						}
						else
						{
							LOG_ERR("Undefined variable {}", name->chars);
							return InterpretResult::RUNTIME_ERROR;
						}
						break;
					}
					case OpCode::DEFINE_GLOBAL:
					{
						ObjString* name = READ_STRING();
						globals[std::string(name->chars)] = Peek(0);
						Pop();
						break;
					}
					case OpCode::SET_GLOBAL:
					{
						ObjString* name = READ_STRING();
						if (auto search = globals.find(std::string(name->chars)); search != globals.end())
						{
							search->second = Peek(0); // Don't pop because assignment is an expression
						}
						else
						{
							// Variable not set
							LOG_ERR("Undefined variable {}", name->chars);
							return InterpretResult::RUNTIME_ERROR;
						}
						break;
					}
					case OpCode::EQUAL:
					{
						DynamicValue b = Pop();
						DynamicValue a = Pop();
						Push(a == b);
						break;
					}
					case OpCode::GREATER: BINARY_OP(>); break;
					case OpCode::LESS: BINARY_OP(<); break;
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
					case OpCode::PRINT:
					{
						PrintValue(Pop());
						std::cout << std::endl;
						break;
					}
					case OpCode::RETURN:
					{
						return InterpretResult::OK;
					}
					default: ASSERT(false);
				}
			}
#undef READ_BYTE
#undef READ_OPCODE
#undef READ_CONSTANT
#undef READ_STRING
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
			stackTop = &stack[0];
		}

		Chunk* chunk;
		U8* ip;
		DynamicValue stack[STACK_MAX];
		DynamicValue* stackTop;
		Obj* objects;
		std::unordered_map<std::string, DynamicValue> globals;
	};
}
