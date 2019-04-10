#pragma once

#include "third_party.h"

namespace djinn::graphics {
    struct QueueFamilyIndices {
        int m_GraphicsFamilyIdx = -1;
        int m_PresentFamilyIdx  = -1;
        int m_TransferFamilyIdx = -1;

		bool isComplete() const;

		static QueueFamilyIndices findQueueFamilyIndices(
			const vk::PhysicalDevice& gpu, 
			const vk::SurfaceKHR&     surface
		);
    };
}
