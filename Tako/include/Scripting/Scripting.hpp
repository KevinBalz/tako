#pragma once
#include <string_view>
#include "Chunk.hpp"
#include "VM.hpp"

namespace tako::Scripting
{
	void Run(std::string_view source)
	{
		VM vm;
		vm.Init();
		vm.Interpret(source);
	}
}
