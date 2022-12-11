#pragma once
#include <vector>

namespace tako::Scripting
{
	enum class ScriptType
	{
		Number,
		Bool,
		String,
		Function,
		NativeFunction,
		Nil
	};

	class Interpreter;
	class ScriptValue;
	class ScriptFunction
	{
	public:
		ScriptFunction(const FunctionDeclaration* fun)
		{
			declaration = fun;
		}

		ScriptValue operator()(Interpreter* interpreter, std::vector<ScriptValue>& arguments);
		const FunctionDeclaration* declaration;
	};

	using ScriptNativeCallback = ScriptValue(*)(Interpreter* interpreter, std::vector<ScriptValue>&);

	class ScriptValue
	{
	public:
		union
		{
			float number;
			bool boolean;
			std::string str;
			ScriptFunction func;
			ScriptNativeCallback nativeFunc;
		};
		ScriptType type;

		ScriptValue()
		{
			number = 0;
			type = ScriptType::Nil;
		}

		ScriptValue(bool b)
		{
			boolean = b;
			type = ScriptType::Bool;
		}

		ScriptValue(float n)
		{
			number = n;
			type = ScriptType::Number;
		}

		ScriptValue(std::string s)
		{
			new (&str) std::string(std::move(s));
			type = ScriptType::String;
		}

		ScriptValue(const FunctionDeclaration* fun)
		{
			func = fun;
			type = ScriptType::Function;
		}

		ScriptValue(ScriptNativeCallback func)
		{
			nativeFunc = func;
			type = ScriptType::NativeFunction;
		}

		~ScriptValue()
		{
			switch (type)
			{
				case ScriptType::String:
					std::destroy_at(&str);
					break;
			}
		}

		ScriptValue(const ScriptValue& other)
		{
			CopyInit(other);
		}

		ScriptValue& operator=(const ScriptValue& other)
		{
			if (type != other.type)
			{
				std::destroy_at(this);
				CopyInit(other);
				return *this;
			}
			switch (type)
			{
				case ScriptType::String:
					str = other.str;
					break;
				default:
					CopyInit(other);
			}
			return *this;
		}

		bool IsTruthy()
		{
			if (type == ScriptType::Nil) return false;
			if (type == ScriptType::Bool) return boolean;
			return true;
		}

		bool operator==(const ScriptValue& other)
		{
			if (type != other.type) return false;
			switch (type)
			{
				case ScriptType::Nil: return true;
				case ScriptType::Number: return number == other.number;
				case ScriptType::Bool: return boolean == other.boolean;
				case ScriptType::String: return str == other.str;
				case ScriptType::Function: return func.declaration == other.func.declaration;
				case ScriptType::NativeFunction: return nativeFunc == other.nativeFunc;
			}
		}

		ScriptValue operator()(Interpreter* interpreter, std::vector<ScriptValue>& arguments)
		{
			switch (type)
			{
				case ScriptType::Function:
					return func(interpreter, arguments);
				case ScriptType::NativeFunction:
					return nativeFunc(interpreter, arguments);
			}
			// Error
			return ScriptValue();
		}

		void Print()
		{
			switch (type)
			{
				case ScriptType::Nil:
					LOG("nil");
					break;
				case ScriptType::Number:
					LOG("{}", number);
					break;
				case ScriptType::Bool:
					LOG("{}", boolean);
					break;
				case ScriptType::String:
					LOG("{}", str);
					break;
			}
		}
	private:
		void CopyInit(const ScriptValue& other)
		{
			type = other.type;
			switch (other.type)
			{
				case ScriptType::Nil:
					number = 0;
					break;
				case ScriptType::Number:
					number = other.number;
					break;
				case ScriptType::Bool:
					boolean = other.boolean;
					break;
				case ScriptType::String:
					new (&str) std::string(other.str);
					break;
				case ScriptType::Function:
					func = other.func;
					break;
				case ScriptType::NativeFunction:
					nativeFunc = other.nativeFunc;
					break;
			}
		}
	};

	ScriptValue operator+(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
		case ScriptType::Number:
			return ScriptValue(a.number + b.number);
		case ScriptType::String:
			return ScriptValue(a.str + b.str);
		}
	}

	ScriptValue operator-(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
		case ScriptType::Number:
			return ScriptValue(a.number - b.number);
		}
	}

	ScriptValue operator*(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
		case ScriptType::Number:
			return ScriptValue(a.number * b.number);
		}
	}

	ScriptValue operator/(const ScriptValue& a, const ScriptValue& b)
	{
		ASSERT(a.type == b.type);
		switch (a.type)
		{
		case ScriptType::Number:
			return ScriptValue(a.number / b.number);
		}
	}
}
