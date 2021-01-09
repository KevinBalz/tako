#pragma once
#include "Tako.hpp"
#include "wren.hpp"

namespace tako
{
	class WrenScriptContext
	{
	public:
		WrenScriptContext();
		~WrenScriptContext();

		void Load(const char* file);

		void Setup(PixelArtDrawer* drawer, Resources* resources);
		void Update(Input* input, float dt);
		void Draw(PixelArtDrawer* drawer);
	private:
		WrenHandle* m_setupHandle = nullptr;
		WrenHandle* m_updateHandle = nullptr;
		WrenHandle* m_drawHandle = nullptr;
		WrenHandle* m_drawerHandle = nullptr;
		WrenVM* m_vm;

		static WrenForeignMethodFn BindForeignMethodFn(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature);
		static WrenForeignClassMethods BindForeignClass(WrenVM* vm, const char* module, const char* className);
		static void SetSetup(WrenVM* vm);
		static void SetUpdate(WrenVM* vm);
		static void SetDraw(WrenVM* vm);

		static void NewColor(WrenVM* vm);

		static void DrawRect(WrenVM* vm);
	};
}