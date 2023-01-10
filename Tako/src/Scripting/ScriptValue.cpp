#include "Scripting/ScriptValue.hpp"
#include "Scripting/Object.hpp"

namespace tako::Scripting
{
	bool DynamicValue::IsString()
	{
		return IsObjType(ObjType::String);
	}

	bool DynamicValue::IsFunction()
	{
		return IsObjType(ObjType::Function);
	}

	bool DynamicValue::IsObjType(ObjType objType)
	{
		return IsObj() && as.obj->type == objType;
	}

	bool DynamicValue::operator==(const DynamicValue& other)
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
					case ObjType::Function:
					{
						auto func = static_cast<ObjFunction*>(obj);
						if (!func->name)
						{
							os << "<script>";
							break;
						}
						os << "<fn " << func->name->chars << ">";
						break;
					}
					case ObjType::String:
					{
						os << static_cast<ObjString*>(obj)->chars;
						break;
					}
				}
				break;
			}
		}
		return os;
	}

	void PrintValue(DynamicValue value, bool showStrParen)
	{
		bool showParen = showStrParen && value.IsString();
		if (showParen)
		{
			std::cout << "\"";
		}
		std::cout << value;
		if (showParen)
		{
			std::cout << "\"";
		}
	}
}
