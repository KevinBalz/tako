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
		VM vm;
		vm.Init();
		vm.Interpret(source);
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
