#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include "ScriptValue.hpp"

namespace tako::Scripting
{
	namespace
	{
		struct string_hash
		{
			using is_transparent = void;

			[[nodiscard]] size_t operator()(const char* str) const
			{
				return std::hash<std::string_view>{}(str);
			}

			[[nodiscard]] size_t operator()(std::string_view str) const
			{
				return std::hash<std::string_view>{}(str);
			}

			[[nodiscard]] size_t operator()(const std::string& str) const
			{
				return std::hash<std::string>{}(str);
			}
		};
	}

	class Environment
	{
	public:
		Environment(std::shared_ptr<Environment> parent = nullptr)
		{
			m_parent = parent;
		}

		void Define(const std::string& name, const ScriptValue& value)
		{
			values[name] = value;
		}

		void Define(std::string_view name, const ScriptValue& value)
		{
			values[std::string(name)] = value;
		}

		void Define(const char* name, const ScriptValue& value)
		{
			values[std::string(name)] = value;
		}

		void Assign(std::string_view name, const ScriptValue& value)
		{
			if (auto search = values.find(name); search != values.end())
			{
				search->second = value;
				return;
			}

			if (m_parent)
			{
				m_parent->Assign(name, value);
				return;
			}

			// Error
		}

		ScriptValue Get(std::string_view name)
		{
			if (auto search = values.find(name); search != values.end())
			{
				return search->second;
			}

			if (m_parent)
			{
				return m_parent->Get(name);
			}

			return ScriptValue();
		}
	private:
		std::shared_ptr<Environment> m_parent;
		std::unordered_map<std::string, ScriptValue, string_hash, std::equal_to<>> values;
	};
}
