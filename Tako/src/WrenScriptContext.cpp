#include "WrenScriptContext.hpp"
#include "Utility.hpp"

template<typename T>
auto WrenGet(WrenVM* vm, int slot)
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
		return static_cast<T*>(wrenGetSlotForeign(vm, slot));
	}
	else
	{
		ASSERT(false);
	}
}

template<typename T>
auto WrenGetLiteral(WrenVM* vm, int slot)
{
	if constexpr (std::is_same_v<bool, T>)
	{
		LOG("slot {} {}", slot, "bool");
		return wrenGetSlotBool(vm, slot);
	}
	else if constexpr (std::is_convertible_v<double, T>)
	{
		LOG("slot {} {}", slot, "number");
		return static_cast<T>(wrenGetSlotDouble(vm, slot));
	}
	else if constexpr (std::is_class_v<T>)
	{
		LOG("slot {} {}", slot, "class");
		return *static_cast<T*>(wrenGetSlotForeign(vm, slot));
	}
	else
	{
		ASSERT(false);
	}
}

template<typename T>
void WrenSet(WrenVM* vm, int slot, T value)
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
	typedef std::tuple<Args...> args;
	typedef std::make_index_sequence<sizeof...(Args)> argSequence;
};

template<typename T> using ClassMethodPtrClass = typename MemberPtrTraits<T>::type;
template<typename T> using ClassMethodPtrReturn = typename MemberPtrTraits<T>::ret;
template<typename T, size_t N> using ClassMethodPtrArg = typename std::tuple_element<N, typename MemberPtrTraits<T>::args>::type;
template<typename T> using ClassMethodPtrArgsSeq = typename MemberPtrTraits<T>::argSequence;


template<auto fPtr>
void WrenFieldGetter(WrenVM* vm)
{
	auto value = WrenGet<ClassFieldPtrClass<decltype(fPtr)>>(vm, 0);
	WrenSet(vm, 0, value->*fPtr);
}

template<auto fPtr>
void WrenFieldSetter(WrenVM* vm)
{
	auto obj = WrenGet<ClassFieldPtrClass<decltype(fPtr)>>(vm, 0);
	obj->*fPtr = WrenGet<ClassFieldPtrType<decltype(fPtr)>>(vm, 1);
}

template<typename T, typename R, auto mPtr, std::size_t... I>
void WrenMethodCaller(WrenVM* vm, std::index_sequence<I...>)
{
	auto obj = WrenGet<T>(vm, 0);
	if constexpr (std::is_void_v<R>)
	{
		(obj->*mPtr)(WrenGetLiteral<ClassMethodPtrArg<decltype(mPtr),I>>(vm, I+1)...);
	}
	else
	{
		auto ret = (obj->*mPtr)(WrenGetLiteral<ClassMethodPtrArg<decltype(mPtr),I>>(vm, I+1)...);
		WrenSet<R>(vm, ret);
	}
}

template<auto mPtr>
void WrenMethodCaller(WrenVM* vm)
{
	WrenMethodCaller<
		ClassMethodPtrClass<decltype(mPtr)>,
		ClassMethodPtrReturn<decltype(mPtr)>,
		mPtr
	>(vm, ClassMethodPtrArgsSeq<decltype(mPtr)>());
}

const char* TAKO_WREN_DEF =
R"WREN(
	foreign class Drawer {
		foreign drawRect(x, y, w, h, c)
		foreign clear()
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

bool WrenScriptContext::MethodKey::operator<(const WrenScriptContext::MethodKey& other) const
{
	if (module != other.module)
	{
		return module < other.module;
	}
	if (className != other.className)
	{
		return className < other.className;
	}
	if (isStatic != other.isStatic)
	{
		return isStatic;
	}
	return signature < other.signature;
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

	m_foreignMap[{"tako", "Tako", true, "setup(_)"}] = WrenScriptContext::SetSetup;
	m_foreignMap[{"tako", "Tako", true, "update(_)"}] = WrenScriptContext::SetUpdate;
	m_foreignMap[{"tako", "Tako", true, "draw(_)"}] = WrenScriptContext::SetDraw;
	m_foreignMap[{"tako", "Drawer", false, "drawRect(_,_,_,_,_)"}] = WrenScriptContext::DrawRect;
	m_foreignMap[{"tako", "Drawer", false, "clear()"}] = WrenMethodCaller<&PixelArtDrawer::Clear>;
	m_foreignMap[{"tako", "Color", false, "init new(_,_,_,_)"}] = [](WrenVM* vm){};
	m_foreignMap[{"tako", "Color", false, "r"}] = WrenFieldGetter<&Color::r>;
	m_foreignMap[{"tako", "Color", false, "g"}] = WrenFieldGetter<&Color::g>;
	m_foreignMap[{"tako", "Color", false, "b"}] = WrenFieldGetter<&Color::b>;
	m_foreignMap[{"tako", "Color", false, "a"}] = WrenFieldGetter<&Color::a>;
	m_foreignMap[{"tako", "Color", false, "r=(_)"}] = WrenFieldSetter<&Color::r>;
	m_foreignMap[{"tako", "Color", false, "g=(_)"}] = WrenFieldSetter<&Color::g>;
	m_foreignMap[{"tako", "Color", false, "b=(_)"}] = WrenFieldSetter<&Color::b>;
	m_foreignMap[{"tako", "Color", false, "a=(_)"}] = WrenFieldSetter<&Color::a>;

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

WrenForeignMethodFn WrenScriptContext::BindForeignMethodFn(WrenVM* vm, const char* module, const char *className, bool isStatic, const char* signature)
{
	auto context = static_cast<WrenScriptContext*>(wrenGetUserData(vm));
	return context->m_foreignMap[{module, className, isStatic, signature}];
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
