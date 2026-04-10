#include "Slope.h"
#include <algorithm>
#include <cmath>

Slope::Slope(const std::shared_ptr<Util::GameObject>& image,
             const glm::vec2& start,
             const glm::vec2& end,
             float slideSpeed,
             float moveFactor,
             bool isSolid)
    : m_Image(image),
      m_Start(start),
      m_End(end),
      m_SlideSpeed(slideSpeed),
      m_MoveFactor(moveFactor),
      m_IsSolid(isSolid) {
}

float Slope::GetSurfaceY(float x) const {
    float dx = m_End.x - m_Start.x;

    if (std::abs(dx) < 0.0001f) {
        return m_Start.y;
    }

    float t = (x - m_Start.x) / dx;
    return m_Start.y + t * (m_End.y - m_Start.y);
}

bool Slope::IsOnSlope(const std::shared_ptr<Util::GameObject>& player,
                      float footOffset,
                      float tolerance) const {
    if (!m_IsSolid) {
        return false;
    }

    float footX = player->m_Transform.translation.x;
    float footY = player->m_Transform.translation.y - footOffset;

    float minX = std::min(m_Start.x, m_End.x);
    float maxX = std::max(m_Start.x, m_End.x);

    if (footX < minX || footX > maxX) {
        return false;
    }

    float surfaceY = GetSurfaceY(footX);
    return std::abs(footY - surfaceY) <= tolerance;
}

void Slope::SnapToSlope(const std::shared_ptr<Util::GameObject>& player,
                        float footOffset) const {
    float x = player->m_Transform.translation.x;
    float surfaceY = GetSurfaceY(x);
    player->m_Transform.translation.y = surfaceY + footOffset;
}

bool Slope::IsLeftLowRightHigh() const {
    return m_Start.y < m_End.y;
}