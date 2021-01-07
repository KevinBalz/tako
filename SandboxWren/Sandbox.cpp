#include "Tako.hpp"
#include "wren.hpp"

void writeFn(WrenVM* vm, const char* text)
{
	LOG("{}", text);
}

struct WrenUserData
{
	tako::PixelArtDrawer* drawer;
	WrenHandle* updateHandle;
	WrenHandle* drawerHandle;
};

void setUpdate(WrenVM* vm)
{
	WrenUserData* data = (WrenUserData*) wrenGetUserData(vm);
	if (data->updateHandle)
	{
		wrenReleaseHandle(vm, data->updateHandle);
	}
	data->updateHandle = wrenGetSlotHandle(vm, 1);
}

void setDraw(WrenVM* vm)
{
	WrenUserData* data = (WrenUserData*) wrenGetUserData(vm);
	if (data->drawerHandle)
	{
		wrenReleaseHandle(vm, data->drawerHandle);
	}
	data->drawerHandle = wrenGetSlotHandle(vm, 1);
}

void draw(WrenVM* vm)
{
	double x = wrenGetSlotDouble(vm, 1);
	double y = wrenGetSlotDouble(vm, 2);
	double w = wrenGetSlotDouble(vm, 3);
	double h = wrenGetSlotDouble(vm, 4);

	((WrenUserData*)wrenGetUserData(vm))->drawer->DrawRectangle(x, y, w, h, {255, 0, 0, 255});
}

WrenForeignMethodFn bindForeignMethod(
		WrenVM* vm,
		const char* module,
		const char* className,
		bool isStatic,
		const char* signature)
{
	if (strcmp(module, "tako") == 0)
	{
		if (strcmp(className, "Tako") == 0)
		{
			if (isStatic && strcmp(signature, "update(_)") == 0)
			{
				return setUpdate; // C function for Math.add(_,_).
			}
			if (isStatic && strcmp(signature, "draw(_)") == 0)
			{
				return setDraw; // C function for Math.add(_,_).
			}
			if (isStatic && strcmp(signature, "drawRect(_,_,_,_)") == 0)
			{
				return draw; // C function for Math.add(_,_).
			}
			// Other foreign methods on Math...
		}
		// Other classes in main...
	}
	// Other modules...
}

WrenVM* vm;
WrenUserData g_data = {};

const char* code =
R"WREN(
	class Tako {
		foreign static update(fn)
		foreign static draw(fn)
		foreign static drawRect(x, y, w, h)
	}
)WREN";

void tako::Setup(tako::PixelArtDrawer* drawer, tako::Resources* resources)
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = writeFn;
	config.bindForeignMethodFn = bindForeignMethod;
	vm = wrenNewVM(&config);

	g_data.drawer = drawer;
	wrenSetUserData(vm, &g_data);

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
			vm,
			"tako",
			code
	);
	result = wrenInterpret
	(
		vm,
		"main",
		mainCode
	);
}
void tako::Update(tako::Input* input, float dt)
{
}

void tako::Draw(tako::PixelArtDrawer* drawer)
{
	drawer->Clear();
	wrenCall(vm, g_data.drawerHandle);
}
