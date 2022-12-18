#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "Utility.hpp"
#include "Object.hpp"

namespace tako::Scripting
{
	enum class DynamicType
	{
		Nil,
		Bool,
		Number,
		Obj
	};

	//struct Obj;
	//struct ObjString;

	struct DynamicValue
	{
		DynamicType type;
		union
		{
			bool boolean;
			double number;
			Obj* obj;
		} as;

		DynamicValue()
		{
			type = DynamicType::Nil;
			as.boolean = 0;
		}

		DynamicValue(bool b)
		{
			type = DynamicType::Bool;
			as.boolean = b;
		}

		DynamicValue(double d)
		{
			type = DynamicType::Number;
			as.number = d;
		}

		DynamicValue(Obj* obj)
		{
			type = DynamicType::Obj;
			as.obj = obj;
		}

		bool IsTruthy()
		{
			return !IsNil() && (!IsBool() || as.boolean);
		}

		bool IsNil()
		{
			return type == DynamicType::Nil;
		}

		bool IsBool()
		{
			return type == DynamicType::Bool;
		}

		bool IsNumber()
		{
			return type == DynamicType::Number;
		}

		bool IsString()
		{
			return IsObjType(ObjType::String);
		}

		bool IsObj()
		{
			return type == DynamicType::Obj;
		}

		bool IsObjType(ObjType objType)
		{
			return IsObj() && as.obj->type == objType;
		}

		bool operator==(const DynamicValue& other)
		{
			if (type != other.type) return false;
			switch (type)
			{
				case DynamicType::Nil: return true;
				case DynamicType::Number: return as.number == other.as.number;
				case DynamicType::Bool: return as.boolean == other.as.boolean;
				case DynamicType::Obj:
					if (as.obj->type != other.as.obj->type)
					{
						return false;
					}
					if (as.obj == other.as.obj)
					{
						return true;
					}

					switch (as.obj->type)
					{
						case ObjType::String:
						{
							ObjString* a = static_cast<ObjString*>(as.obj);
							ObjString* b = static_cast<ObjString*>(other.as.obj);

							return a->length == b->length && memcmp(a->chars, b->chars, a->length) == 0;
						}
					}
					break;
				default: ASSERT(false);
			}
		}
	};

	std::ostream& operator<<(std::ostream& os, const DynamicValue& val)
	{
		switch (val.type)
		{
			case DynamicType::Nil:
				os << "nil";
				break;
			case DynamicType::Number:
				os << val.as.number;
				break;
			case DynamicType::Bool:
				os << (val.as.boolean ? "true" : "false");
				break;
			case DynamicType::Obj:
			{
				auto obj = val.as.obj;
				switch (obj->type)
				{
					case ObjType::String:
						os << static_cast<ObjString*>(obj)->chars;
						break;
				}
				break;
			}
		}
		return os;
	}

	void PrintValue(DynamicValue value)
	{
		std::cout << value;
	}
	/*
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
	class Environment;
	struct FunctionDeclaration;

	class ScriptFunction
	{
	public:
		ScriptFunction(const FunctionDeclaration* fun, std::shared_ptr<Environment> closure)
		{
			declaration = fun;
			this->closure = closure;
		}

		ScriptValue operator()(Interpreter* interpreter, std::vector<ScriptValue>& arguments);
		const FunctionDeclaration* declaration;
		std::shared_ptr<Environment> closure;
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

		ScriptValue(const FunctionDeclaration* fun, std::shared_ptr<Environment> closure)
		{
			new (&func) ScriptFunction(fun, closure);
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
				case ScriptType::Function:
					std::destroy_at(&func);
					break;
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
				case ScriptType::Function:
					func = other.func;
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
					new (&func) ScriptFunction(other.func);
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
	*/
}
