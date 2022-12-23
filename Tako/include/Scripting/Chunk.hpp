#pragma once
#include "NumberTypes.hpp"
#include "ScriptValue.hpp"
#include <vector>
#include <iostream>

namespace tako::Scripting
{
	enum class OpCode : U8
	{
		CONSTANT,
		NIL,
		TRUE,
		FALSE,
		POP,
		GET_LOCAL,
		SET_LOCAL,
		GET_GLOBAL,
		DEFINE_GLOBAL,
		SET_GLOBAL,
		EQUAL,
		GREATER,
		LESS,
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		NOT,
		NEGATE,
		PRINT,
		RETURN,
	};

	class Chunk
	{
	public:

		void Write(U8 byte, int line)
		{
			code.push_back(byte);
			lines.push_back(line);
		}

		inline void Write(OpCode code, int line)
		{
			Write((U8) code, line);
		}

		int AddConstant(DynamicValue value)
		{
			constants.push_back(value);
			return constants.size() - 1;
		}

		void Disassemble(std::string_view name)
		{
			std::cout << "==" << name << "==" << "\n";

			for (int offset = 0; offset < code.size();)
			{
				offset = DisassembleInstruction(offset);
			}
		}

		int DisassembleInstruction(int offset)
		{
			std::cout << offset << " "; // %04d

			if (offset > 0 && lines[offset] == lines[offset - 1])
			{
				std::cout << "   | ";
			}
			else
			{
				std::cout << lines[offset]; // %4d;
			}

			OpCode instruction = (OpCode) code[offset];
			switch (instruction)
			{
				case OpCode::CONSTANT:
					return ConstantInstruction("OP_CONSTANT", offset);
				case OpCode::NIL:
					return SimpleInstruction("OP_NIL", offset);
				case OpCode::TRUE:
					return SimpleInstruction("OP_TRUE", offset);
				case OpCode::FALSE:
					return SimpleInstruction("OP_FALSE", offset);
				case OpCode::POP:
					return SimpleInstruction("OP_POP", offset);
				case OpCode::GET_LOCAL:
					return ByteInstruction("OP_GET_LOCAL", offset);
				case OpCode::SET_LOCAL:
					return ByteInstruction("OP_SET_LOCAL", offset);
				case OpCode::GET_GLOBAL:
					return ConstantInstruction("OP_GET_GLOBAL", offset);
				case OpCode::DEFINE_GLOBAL:
					return ConstantInstruction("OP_DEFINE_GLOBAL", offset);
				case OpCode::SET_GLOBAL:
					return ConstantInstruction("OP_SET_GLOBAL", offset);
				case OpCode::EQUAL:
					return SimpleInstruction("OP_EQUAL", offset);
				case OpCode::GREATER:
					return SimpleInstruction("OP_GREATER", offset);
				case OpCode::LESS:
					return SimpleInstruction("OP_LESS", offset);
				case OpCode::ADD:
					return SimpleInstruction("OP_ADD", offset);
				case OpCode::SUBTRACT:
					return SimpleInstruction("OP_SUBTRACT", offset);
				case OpCode::MULTIPLY:
					return SimpleInstruction("OP_MULTIPLY", offset);
				case OpCode::DIVIDE:
					return SimpleInstruction("OP_DIVIDE", offset);
				case OpCode::NOT:
					return SimpleInstruction("OP_NOT", offset);
				case OpCode::NEGATE:
					return SimpleInstruction("OP_NEGATE", offset);
				case OpCode::PRINT:
					return SimpleInstruction("OP_PRINT", offset);
				case OpCode::RETURN:
					return SimpleInstruction("OP_RETURN", offset);
				default:
					std::cout << "Unknown opcode " << (U8) instruction << "\n";
					return offset + 1;
			}
		}

		int SimpleInstruction(std::string_view name, int offset)
		{
			std::cout << name << "\n";
			return offset + 1;
		}

		int ByteInstruction(std::string_view name, int offset)
		{
			U8 slot = code[offset + 1];
			std::cout << name << " " << (int) slot << "\n";
			return offset + 2;
		}

		int ConstantInstruction(std::string_view name, int offset)
		{
			U8 constant = code[offset + 1];
			std::cout << name << " " << (int) constant << " "; //%4d
			PrintValue(constants[constant]);
			std::cout << "\n";
			return offset + 2;
		}

		std::vector<U8> code;
		std::vector<int> lines;
		std::vector<DynamicValue> constants;
	};
}
