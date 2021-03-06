#pragma once

#include "Math.hpp"
#include "Bitmap.hpp"
#include "Texture.hpp"
#include "Sprite.hpp"
#include "GraphicsContext.hpp"

namespace tako
{
	class PixelArtDrawer
	{
	public:
		PixelArtDrawer(GraphicsContext* context) : m_context(context) {}
		virtual ~PixelArtDrawer() {}

		virtual void SetClearColor(Color c) = 0;
		virtual void SetTargetSize(int width, int height) = 0;
		virtual void AutoScale() = 0;
		virtual void SetCameraPosition(Vector2 position) = 0;
		virtual Vector2 GetCameraPosition() = 0;
		virtual Vector2 GetCameraViewSize() = 0;

		virtual void Clear() = 0;
		virtual void DrawRectangle(float x, float y, float w, float h, Color c) = 0;
		virtual void DrawImage(float x, float y, float w, float h, const TextureHandle img, Color color = {255, 255, 255, 255}) = 0;
		virtual void DrawSprite(float x, float y, float w, float h, const Sprite* sprite, Color color = {255, 255, 255, 255}) = 0;

		virtual Texture CreateTexture(const Bitmap& bitmap) = 0;
		virtual Sprite* CreateSprite(const Texture texture, float x, float y, float w, float h) = 0;

		virtual void UpdateTexture(Texture texture, const Bitmap& bitmap) = 0;
	protected:
		GraphicsContext* m_context;
	};
}
