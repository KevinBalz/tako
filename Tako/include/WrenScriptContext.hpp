#pragma once
#include <map>
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

		struct MethodKey
		{
			std::string module;
			std::string className;
			bool isStatic;
			std::string signature;
			bool operator<(const MethodKey& other) const;
		};

		std::map<MethodKey, WrenForeignMethodFn> m_foreignMap;

		static WrenForeignMethodFn BindForeignMethodFn(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature);
		static WrenForeignClassMethods BindForeignClass(WrenVM* vm, const char* module, const char* className);
		static void SetSetup(WrenVM* vm);
		static void SetUpdate(WrenVM* vm);
		static void SetDraw(WrenVM* vm);
	};
}