#pragma once

#include "renderer.h"
#include "resources/resources.h"

namespace djinn::graphics {
	class ForwardRendererBase:
		public Renderer
	{
	public:
		ForwardRendererBase();
		ForwardRendererBase(
			Graphics* graphics,
			Resources* resources,
			int width,
			int height
		);
	};
}
