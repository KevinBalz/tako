#include "WrenScriptContext.hpp"
#include "Utility.hpp"

const char* TAKO_WREN_DEF =
R"WREN(
	class Tako {
		foreign static setup(fn)
		foreign static update(fn)
		foreign static draw(fn)
		foreign static drawRect(x, y, w, h)
	}
	var a = 5 / 0
)WREN";

namespace tako
{

	void WriteFn(WrenVM* vm, const char* text)
	{
		LOG("{}", text);
	}

	void ErrorFn(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message)
	{
		switch (type)
		{
			case WREN_ERROR_COMPILE:
				LOG_ERR("[WREN] Compilation error in module \"{}\" line {}: {}", module, line, message);
				break;
			case WREN_ERROR_RUNTIME:
				LOG_ERR("[WREN] Runtime error: {}", message);
				break;
			case WREN_ERROR_STACK_TRACE:
				LOG_ERR("LINE {} {}.{}", line, module, message);
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

	void WrenScriptContext::Load(const char* file)
	{
		constexpr size_t bufferSize = 1024 * 1024;
		std::array <tako::U8, bufferSize> buffer;
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

	void WrenScriptContext::Setup(PixelArtDrawer* drawer, Resources* resources)
	{
		if (m_setupHandle)
		{
			WrenInterpretResult result = wrenCall(m_vm, m_setupHandle);
			ASSERT(result == WREN_RESULT_SUCCESS);
		}
	}
	void WrenScriptContext::Update(Input* input, float dt)
	{
		if (m_updateHandle)
		{
			WrenInterpretResult result = wrenCall(m_vm, m_updateHandle);
			ASSERT(result == WREN_RESULT_SUCCESS);
		}
	}
	void WrenScriptContext::Draw(PixelArtDrawer* drawer)
	{
		drawer->Clear();
		m_drawer = drawer;
		if (m_drawHandle)
		{
			WrenInterpretResult result = wrenCall(m_vm, m_drawHandle);
			ASSERT(result == WREN_RESULT_SUCCESS);
		}
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
					if (strcmp(signature, "drawRect(_,_,_,_)") == 0)
					{
						return WrenScriptContext::DrawRect;
					}
				}
			}
		}
		return nullptr;
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
		auto context = static_cast<WrenScriptContext*>(wrenGetUserData(vm));
		double x = wrenGetSlotDouble(vm, 1);
		double y = wrenGetSlotDouble(vm, 2);
		double w = wrenGetSlotDouble(vm, 3);
		double h = wrenGetSlotDouble(vm, 4);

		context->m_drawer->DrawRectangle(x, y, w, h, {255, 0, 0, 255});
	}
}
