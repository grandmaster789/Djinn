#pragma once

namespace djinn {
	class Graphics;
}

namespace djinn::graphics {
	class Renderer
	{
	public:
		Renderer()          = default;
		virtual ~Renderer() = default;

		Renderer             (const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;
		Renderer             (Renderer&&)      = default;
		Renderer& operator = (Renderer&&)      = default;

		Renderer(Graphics* graphics, int width, int height);

		virtual void render() = 0;
		virtual void resize(int newWidth, int newHeight);

		int getWidth()  const noexcept;
		int getHeight() const noexcept;

	protected:
		Graphics* m_Graphics = nullptr;

		int m_Width  = 0;
		int m_Height = 0;
	};
}