#include "queueFamilyIndices.h"

namespace djinn::graphics {
	bool QueueFamilyIndices::isComplete() const {
		return
			(m_GraphicsFamilyIdx >= 0) &&
			(m_PresentFamilyIdx  >= 0) &&
			(m_TransferFamilyIdx >= 0);
	}

	QueueFamilyIndices QueueFamilyIndices::findQueueFamilyIndices(
		const vk::PhysicalDevice& gpu,
		const vk::SurfaceKHR&     surface
	) {
		QueueFamilyIndices result;

		auto props = gpu.getQueueFamilyProperties();

		int idx = 0;

		for (const auto& family : props) {
			auto presentSupport = gpu.getSurfaceSupportKHR(idx, surface);

			if (family.queueCount > 0) {
				if (family.queueFlags & vk::QueueFlagBits::eGraphics)
					result.m_GraphicsFamilyIdx = idx;

				// prefer a dedicated transfer queue
				if (!(family.queueFlags & vk::QueueFlagBits::eGraphics) &&
					 (family.queueFlags & vk::QueueFlagBits::eTransfer)
				)
					result.m_TransferFamilyIdx = idx;

				if (presentSupport)
					result.m_PresentFamilyIdx = idx;
			}

			if (result.isComplete())
				break;

			++idx;
		}

		// if no transfer queue was picked so far, try again without a preference for dedicated queues
		idx = 0;
		if (result.m_TransferFamilyIdx == -1) {
			for (const auto& family : props) {
				if (family.queueFlags & vk::QueueFlagBits::eTransfer) {
					result.m_TransferFamilyIdx = idx;
					break;
				}

				++idx;
			}
		}

		return result;
	}
}