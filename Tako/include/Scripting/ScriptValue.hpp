#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "Utility.hpp"

namespace tako::Scripting
{
	enum class DynamicType
	{
		Nil,
		Bool,
		Number,
		Obj
	};

	struct Obj;
	enum class ObjType;
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

		bool IsString();
		bool IsFunction();

		bool IsObj()
		{
			return type == DynamicType::Obj;
		}

		bool IsObjType(ObjType objType);

		bool operator==(const DynamicValue& other);
	};

	std::ostream& operator<<(std::ostream& os, const DynamicValue& val);

	void PrintValue(DynamicValue value, bool showStrParen = false);
}
