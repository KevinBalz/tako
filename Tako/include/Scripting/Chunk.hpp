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
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		NOT,
		NEGATE,
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

		int ConstantInstruction(std::string_view name, int offset)
		{
			U8 constant = code[offset + 1];
			std::cout << name << " " << constant; //%4d
			PrintValue(constants[constant]);
			std::cout << "\n";
			return offset + 2;
		}

		std::vector<U8> code;
		std::vector<int> lines;
		std::vector<DynamicValue> constants;
	};
}
