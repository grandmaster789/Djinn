#pragma once

namespace djinn::graphics {
    struct QueueFamilyIndices {
        int m_GraphicsFamilyIdx = -1;
        int m_PresentFamilyIdx  = -1;
        int m_TransferFamilyIdx = -1;

        inline bool isComplete() const {
            return
                (m_GraphicsFamilyIdx >= 0) &&
                (m_PresentFamilyIdx  >= 0) &&
                (m_TransferFamilyIdx >= 0);
        }
    };
}
