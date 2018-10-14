#pragma once

#include "dependencies.h"
#include <vector>

namespace djinn {
    class Display;
    class Renderer;
}

namespace djinn::display {
    class Window;
}

namespace djinn::renderer {
    
    class Swapchain {
    public:
        using Window = display::Window;

        Swapchain() = default;
        Swapchain( 
            const Window&   window,  // mostly for access to the Surface, its caps etc
                  Display*  display, // for access to the (physical and logical) devices, and family queue indices
                  Renderer* render   // just for accessing the previous swapchain, if any
        );

        vk::SwapchainKHR getHandle() const;
        
        const vk::Extent2D& getExtent() const;
        bool                hasSameExtent(const vk::Extent2D& extent) const;

        uint32_t                          getNumImages() const;
        const std::vector<vk::Image>&     getImages()    const;
              std::vector<vk::ImageView>  getViews()     const;

    private:
        vk::PresentModeKHR     m_PresentMode = vk::PresentModeKHR::eFifo;
        vk::UniqueSwapchainKHR m_Handle;
        vk::Extent2D           m_Extent;

        uint32_t               m_NumImages;

        std::vector<vk::Image>           m_Images;
        std::vector<vk::UniqueImageView> m_Views;
    };
}
