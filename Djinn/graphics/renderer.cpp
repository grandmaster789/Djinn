#include "renderer.h"
#include "graphics.h"

namespace djinn::graphics {
	Renderer::Renderer(Graphics* gfx, int width, int height) :
		m_Graphics(gfx),
		m_Width   (width),
		m_Height  (height)
	{
	}

	void Renderer::resize(int newWidth, int newHeight)
	{
		m_Width = newWidth;
		m_Height = newHeight;
	}

	int Renderer::getWidth() const noexcept {
		return m_Width;
	}

	int Renderer::getHeight() const noexcept {
		return m_Height;
	}
}