#pragma once
#include <vector>
#include <cstddef>

namespace tako::Reflection
{
	struct TypeInformation
	{
		const char* name;
		size_t size;

		constexpr TypeInformation() {}
		//virtual ~TypeInformation() {}
	};

	struct PrimitiveInformation : public TypeInformation
	{
		constexpr PrimitiveInformation(const char* name, size_t size)
		{
			this->name = name;
			this->size = size;
		}
	};

	template<typename T>
	const PrimitiveInformation* GetPrimitiveInformation();

	struct StructInformation : public TypeInformation
	{
		constexpr StructInformation(void (*init)(StructInformation*))
		{
			init(this);
		}

		struct Field
		{
			const char* name;
			size_t offset;
			const TypeInformation* type;
		};

		std::vector<Field> fields;
		void (*constr)(void*);
	};

	struct Resolver
	{
		template<typename T>
		static char func(decltype(&T::Reflection));
		template<typename T>
		static int func(...);

		template<typename T>
		struct IsReflected
		{
			enum { value = (sizeof(func<T>(nullptr)) == sizeof(char)) };
		};

		template<typename T>
		static constexpr auto Get()
		{
			if constexpr (IsReflected<T>::value)
			{
				return &T::Reflection;
			}
			else
			{
				return GetPrimitiveInformation<T>();
			}
		}

		template<typename T>
		static constexpr const char* GetName()
		{
			return Get<T>()->name;
		}
	};

	template<typename T>
	static void InitDefault(void* data)
	{
		new (data) T();
	}

	template<class T, typename F>
	struct MemberPtr
	{
		const char* name;
		F T::* memberPtr;
	};
}

// foreach macro: https://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros
#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X),FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X),FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X),FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X),FE_4(WHAT, __VA_ARGS__)

#define GET_MACRO(_0,_1,_2,_3,_4,_5,NAME,...) NAME
#define FOR_EACH(action,...) \
  GET_MACRO(_0,__VA_ARGS__,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__)

#define REFLECT_GET_FIELD_TYPE(name) decltype(name)

#define REFLECT_MEMBER_PTR(name) (::tako::Reflection::MemberPtr{#name,&SelfType::name})

#define REFLECT_GENERATE_TUPLE(...) using ReflectionTypeList = std::tuple<FOR_EACH(REFLECT_GET_FIELD_TYPE, __VA_ARGS__)>;
#define REFLECT_GENERATE_OFFSET_PTRS(...) static constexpr auto ReflectionMemberPtrs = std::make_tuple(FOR_EACH(REFLECT_MEMBER_PTR, __VA_ARGS__));

#define REFLECT_FIELD(name) {#name, offsetof(SelfType, name), ::tako::Reflection::Resolver::Get<decltype(SelfType::name)>()}

#define REFLECT(type,...) \
	using SelfType = type; \
	friend struct ::tako::Reflection::Resolver; \
	REFLECT_GENERATE_TUPLE(__VA_ARGS__) \
	REFLECT_GENERATE_OFFSET_PTRS(__VA_ARGS__) \
	static void InitReflection(::tako::Reflection::StructInformation* info) \
	{ \
		info->name = #type; \
		info->size = sizeof(SelfType); \
		info->constr = &::tako::Reflection::InitDefault<SelfType>; \
		info->fields = \
		{ \
			FOR_EACH(REFLECT_FIELD,__VA_ARGS__) \
		}; \
	} \
	static inline const ::tako::Reflection::StructInformation Reflection = {InitReflection};
