#pragma once
#include "Input.hpp"
#include "Resources.hpp"

namespace tako
{
	struct SetupData
	{
		GraphicsContext* context;
		Resources* resources;
		Audio* audio;
	};

	struct GameConfig
	{
		void (*Setup)(void* gameData, const SetupData& setup);
		void (*Update)(void* gameData, Input* input, float dt);
		void (*Draw)(void* gameData);
		size_t gameDataSize;
		GraphicsAPI graphicsAPI = GraphicsAPI::Default;
	};
}
