#pragma once
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "Utility.hpp"

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
		Scanner scanner;
		auto tokens = scanner.ScanTokens(source);
		//LOG("{}", tokens.size());
		Parser parser(tokens);
		auto prog = parser.Parse();
		Interpreter interpreter;
		interpreter.Interpret(prog);
		//LOG("val: {}", val);
	}
}
