#include "WrenScriptContext.hpp"
#include "Utility.hpp"

const char* TAKO_WREN_DEF =
R"WREN(
	foreign class Drawer {
		foreign drawRect(x, y, w, h, c)
	}
	foreign class Color {
		foreign construct new(r, g, b, a)
		foreign r
		foreign r=(value)
		foreign g
		foreign g=(value)
		foreign b
		foreign b=(value)
		foreign a
		foreign a=(value)
	}
	class Tako {
		foreign static setup(fn)
		foreign static update(fn)
		foreign static draw(fn)
	}
)WREN";

namespace tako
{

void WriteFn(WrenVM *vm, const char *text)
{
	LOG("{}", text);
}

void ErrorFn(WrenVM *vm, WrenErrorType type, const char *module, int line, const char *message)
{
	switch (type)
	{
		case WREN_ERROR_COMPILE: LOG_ERR("[WREN] Compilation error in module \"{}\" line {}: {}", module, line, message);
			break;
		case WREN_ERROR_RUNTIME: LOG_ERR("[WREN] Runtime error: {}", message);
			break;
		case WREN_ERROR_STACK_TRACE: LOG_ERR("LINE {} {}.{}", line, module, message);
			break;
	}
}

WrenScriptContext::WrenScriptContext()
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = WriteFn;
	config.errorFn = ErrorFn;
	config.bindForeignMethodFn = WrenScriptContext::BindForeignMethodFn;
	config.bindForeignClassFn = WrenScriptContext::BindForeignClass;
	m_vm = wrenNewVM(&config);
	wrenSetUserData(m_vm, this);

	WrenInterpretResult result = wrenInterpret
	(
		m_vm,
		"tako",
		TAKO_WREN_DEF
	);
	ASSERT(result == WREN_RESULT_SUCCESS);
}

WrenScriptContext::~WrenScriptContext()
{
	wrenFreeVM(m_vm);
}

void WrenScriptContext::Load(const char *file)
{
	constexpr size_t bufferSize = 1024 * 1024;
	std::array<tako::U8, bufferSize> buffer;
	size_t bytesRead = 0;
	if (!tako::FileSystem::ReadFile("/main.wren", buffer.data(), bufferSize, bytesRead))
	{
		LOG_ERR("Could not read script {}", "/main.wren");
	}
	auto mainCode = reinterpret_cast<const char *>(buffer.data());

	WrenInterpretResult result = wrenInterpret
	(
		m_vm,
		"main",
		mainCode
	);
	ASSERT(result == WREN_RESULT_SUCCESS);
}

void WrenScriptContext::Setup(PixelArtDrawer *drawer, Resources *resources)
{
	wrenEnsureSlots(m_vm, 3);
	wrenGetVariable(m_vm, "tako", "Drawer", 2);
	auto data = static_cast<PixelArtDrawer **>(wrenSetSlotNewForeign(m_vm, 1, 2, sizeof(PixelArtDrawer *)));
	*data = drawer;
	m_drawerHandle = wrenGetSlotHandle(m_vm, 1);
	if (m_setupHandle)
	{
		WrenInterpretResult result = wrenCall(m_vm, m_setupHandle);
		ASSERT(result == WREN_RESULT_SUCCESS);
	}
}
void WrenScriptContext::Update(Input *input, float dt)
{
	if (m_updateHandle)
	{
		auto callHandle = wrenMakeCallHandle(m_vm, "call(_)");
		wrenEnsureSlots(m_vm, 2);
		wrenSetSlotHandle(m_vm, 0, m_updateHandle);
		wrenSetSlotDouble(m_vm, 1, dt);
		WrenInterpretResult result = wrenCall(m_vm, callHandle);
		ASSERT(result == WREN_RESULT_SUCCESS);
	}
}
void WrenScriptContext::Draw(PixelArtDrawer *drawer)
{
	drawer->Clear();
	if (m_drawHandle)
	{
		auto callHandle = wrenMakeCallHandle(m_vm, "call(_)");
		wrenEnsureSlots(m_vm, 2);
		wrenSetSlotHandle(m_vm, 0, m_drawHandle);
		wrenSetSlotHandle(m_vm, 1, m_drawerHandle);
		WrenInterpretResult result = wrenCall(m_vm, callHandle);
		ASSERT(result == WREN_RESULT_SUCCESS);
	}
}

template<typename T>
auto WrenGet(WrenVM *vm, int slot)
{
	if constexpr (std::is_same_v<bool, T>)
	{
		return wrenGetSlotBool(vm, slot);
	}
	else if constexpr (std::is_convertible_v<double, T>)
	{
		return static_cast<T>(wrenGetSlotDouble(vm, slot));
	}
	else if constexpr (std::is_class_v<T>)
	{
		return static_cast<T *>(wrenGetSlotForeign(vm, slot));
	}
	else
	{
		ASSERT(false);
	}
}

template<typename T>
void WrenSet(WrenVM *vm, int slot, T value)
{
	if constexpr (std::is_same_v<bool, T>)
	{
		wrenSetSlotBool(vm, slot, value);
	}
	else if constexpr (std::is_convertible_v<T, double>)
	{
		wrenSetSlotDouble(vm, slot, value);
	}
	/*
	else if constexpr (std::is_class_v<T>)
	{
		return static_cast<T *>(wrenGetSlotForeign(vm, slot));
	}
	*/
	else
	{
		ASSERT(false);
	}
}


template<typename P> struct MemberFieldTraits;
template<typename R, typename T>
struct MemberFieldTraits<R (T::*)>
{
	typedef R ret;
	typedef T type;
};

template<typename T> using ClassFieldPtrClass = typename MemberFieldTraits<T>::type;
template<typename T> using ClassFieldPtrType = typename MemberFieldTraits<T>::ret;

template<typename P> struct MemberPtrTraits;
template<typename R, typename T, typename... Args>
struct MemberPtrTraits<R (T::*)(Args...)>
{
	typedef R ret;
	typedef T type;
};

template<typename T> using ClassMethodPtrClass = typename MemberPtrTraits<T>::type;
template<typename T> using ClassMethodPtrType = typename MemberPtrTraits<T>::ret;

template<auto fPtr>
void WrenFieldGetter(WrenVM* vm)
{
	auto value = WrenGet<ClassFieldPtrClass<decltype(fPtr)>>(vm, 0);
	WrenSet(vm, 0, value->*fPtr);
}

/*
template<typename R, typename T, R T::*ptr>
void WrenFieldGetter(WrenVM* vm)
{
	auto value = WrenGet<T>(vm, 0);
	WrenSet(vm, 0, value->*ptr);
}
 */

template<auto fPtr>
void WrenFieldSetter(WrenVM* vm)
{
	auto obj = WrenGet<ClassFieldPtrClass<decltype(fPtr)>>(vm, 0);
	obj->*fPtr = WrenGet<ClassFieldPtrType<decltype(fPtr)>>(vm, 1);
}

WrenForeignMethodFn WrenScriptContext::BindForeignMethodFn(WrenVM* vm, const char* module, const char *className, bool isStatic, const char* signature)
{
	if (strcmp(module, "tako") == 0)
	{
		if (strcmp(className, "Tako") == 0)
		{
			if (isStatic)
			{
				if (strcmp(signature, "setup(_)") == 0)
				{
					return WrenScriptContext::SetSetup;
				}
				if (strcmp(signature, "update(_)") == 0)
				{
					return WrenScriptContext::SetUpdate;
				}
				if (strcmp(signature, "draw(_)") == 0)
				{
					return WrenScriptContext::SetDraw;
				}
			}
		}
		if (strcmp(className, "Drawer") == 0)
		{
			if (!isStatic)
			{
				if (strcmp(signature, "drawRect(_,_,_,_,_)") == 0)
				{
					return WrenScriptContext::DrawRect;
				}
			}
		}
		if (strcmp(className, "Color") == 0)
		{
			if (!isStatic)
			{
				if (strcmp(signature, "init new(_,_,_,_)") == 0)
				{
					return [](WrenVM* vm){};
				}
				if (strcmp(signature, "r") == 0)
				{
					return WrenFieldGetter<&Color::r>;
				}
				if (strcmp(signature, "g") == 0)
				{
					return WrenFieldGetter<&Color::g>;
				}
				if (strcmp(signature, "b") == 0)
				{
					return WrenFieldGetter<&Color::b>;
				}
				if (strcmp(signature, "a") == 0)
				{
					return WrenFieldGetter<&Color::a>;
				}
				if (strcmp(signature, "r=(_)") == 0)
				{
					return WrenFieldSetter<&Color::r>;
				}
				if (strcmp(signature, "g=(_)") == 0)
				{
					return WrenFieldSetter<&Color::g>;
				}
				if (strcmp(signature, "b=(_)") == 0)
				{
					return WrenFieldSetter<&Color::b>;
				}
				if (strcmp(signature, "a=(_)") == 0)
				{
					return WrenFieldSetter<&Color::a>;
				}
			}
		}
	}
	return nullptr;
}

void WrenScriptContext::NewColor(WrenVM* vm)
{
	auto color = static_cast<Color*>(wrenGetSlotForeign(vm, 0));
	color->r = WrenGet<U8>(vm, 1);
	color->g = WrenGet<U8>(vm, 2);
	color->b = WrenGet<U8>(vm, 3);
	color->a = WrenGet<U8>(vm, 4);
}

template<typename T, typename... Args>
void WrenAllocFuncConstructor(WrenVM* vm)
{
	ASSERT(sizeof...(Args) < wrenGetSlotCount(vm));
	auto data = wrenSetSlotNewForeign(vm, 0, 0, sizeof(T));
	int i = 1;
	new(data) T(WrenGet<Args>(vm, i++)...);
}

WrenForeignClassMethods WrenScriptContext::BindForeignClass(WrenVM* vm, const char* module, const char* className)
{
	if (strcmp(className, "Color") == 0)
	{
		return { WrenAllocFuncConstructor<Color, U8, U8, U8, U8>, nullptr };
	}
	return
	{
		nullptr,
		nullptr
	};
}

void WrenScriptContext::SetSetup(WrenVM* vm)
{
	auto context = static_cast<WrenScriptContext*>(wrenGetUserData(vm));
	if (context->m_setupHandle)
	{
		wrenReleaseHandle(vm, context->m_setupHandle);
	}
	context->m_setupHandle = wrenGetSlotHandle(vm, 1);
}

void WrenScriptContext::SetUpdate(WrenVM* vm)
{
	auto context = static_cast<WrenScriptContext*>(wrenGetUserData(vm));
	if (context->m_updateHandle)
	{
		wrenReleaseHandle(vm, context->m_updateHandle);
	}
	context->m_updateHandle = wrenGetSlotHandle(vm, 1);
}

void WrenScriptContext::SetDraw(WrenVM* vm)
{
	auto context = static_cast<WrenScriptContext*>(wrenGetUserData(vm));
	if (context->m_drawHandle)
	{
		wrenReleaseHandle(vm, context->m_drawHandle);
	}
	context->m_drawHandle = wrenGetSlotHandle(vm, 1);
}

void WrenScriptContext::DrawRect(WrenVM* vm)
{
	auto drawer = *static_cast<PixelArtDrawer**>(wrenGetSlotForeign(vm, 0));
	double x = wrenGetSlotDouble(vm, 1);
	double y = wrenGetSlotDouble(vm, 2);
	double w = wrenGetSlotDouble(vm, 3);
	double h = wrenGetSlotDouble(vm, 4);
	auto c = static_cast<Color*>(wrenGetSlotForeign(vm, 5));

	drawer->DrawRectangle(x, y, w, h, *c);
}

}
