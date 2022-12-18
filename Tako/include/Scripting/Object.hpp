#pragma once

namespace tako::Scripting
{
	enum class ObjType
	{
		String,
	};

	struct Obj
	{
		ObjType type;
		Obj* next;

		virtual ~Obj()
		{}


	protected:
		Obj(ObjType t)
		{
			type = t;
		}
	};

	struct ObjString final : public Obj
	{
		int length;
		char* chars;

		explicit ObjString(std::string_view str) : Obj(ObjType::String)
		{
			length = str.length();
			chars = new char[length + 1];
			memcpy(chars, str.data(), length + 1);
			chars[length] = '\0';
		}

		virtual ~ObjString() override
		{
			delete[] chars;
		}
	};
}