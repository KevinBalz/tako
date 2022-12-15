#pragma once
#include <string_view>
#include "Chunk.hpp"
#include "VM.hpp"

namespace tako::Scripting
{
	/*
	std::string PrintLiteral(const Literal& lit)
	{
	}

	std::string PrintExpression(const Expression& expr)
	{
		std::visit([](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Literal>)
			{
				return PrintLiteral(arg);
			}
			else if constexpr (std::is_same_v<T, UnaryExpression>)
			{
				return 
			}
		}, expr);
	}
	*/

	void Run(std::string_view source)
	{
		Chunk chunk;
		int constant = chunk.AddConstant(1.2);
		chunk.Write(OpCode::CONSTANT, 123);
		chunk.Write(constant, 123);

		constant = chunk.AddConstant(3.4);
		chunk.Write(OpCode::CONSTANT, 123);
		chunk.Write(constant, 123);

		chunk.Write(OpCode::ADD, 123);

		constant = chunk.AddConstant(5.6);
		chunk.Write(OpCode::CONSTANT, 123);
		chunk.Write(constant, 123);

		chunk.Write(OpCode::DIVIDE, 123);
		chunk.Write(OpCode::NEGATE, 123);

		chunk.Write(OpCode::RETURN, 123);
		chunk.Disassemble("test");
		std::cout << "================\n\n";
		VM vm;
		vm.Init();
		vm.Interpret(&chunk);
		/*
		Scanner scanner;
		auto tokens = scanner.ScanTokens(source);
		//LOG("{}", tokens.size());
		Parser parser(tokens);
		auto prog = parser.Parse();
		Interpreter interpreter;
		{
			Resolver resolver;
			resolver.Resolve(prog.declarations);
			std::swap(interpreter.locals, resolver.locals);
		}
		interpreter.Interpret(prog);
		//LOG("val: {}", val);
		*/
	}
}
