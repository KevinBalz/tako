#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include "ScriptValue.hpp"

namespace tako::Scripting
{
	class Environment
	{
	public:
		void Define(std::string name, const ScriptValue& value)
		{
			values[name] = value;
		}

		void Assign(std::string name, const ScriptValue& value)
		{
			if (auto search = values.find(name); search != values.end())
			{
				search->second = value;
				return;
			}

			// Error
		}

		ScriptValue Get(std::string name)
		{
			if (auto search = values.find(name); search != values.end())
			{
				return search->second;
			}

			return ScriptValue();
		}
	private:
		std::unordered_map<std::string, ScriptValue> values;
	};
}
