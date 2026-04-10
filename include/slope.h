#ifndef SLOPE_H
#define SLOPE_H

#include "pch.hpp"
#include "Util/GameObject.hpp"
#include <memory>

class Slope {
public:
    Slope() = default;

    Slope(const std::shared_ptr<Util::GameObject>& image,
          const glm::vec2& start,
          const glm::vec2& end,
          float slideSpeed,
          float moveFactor,
          bool isSolid);

    float GetSurfaceY(float x) const;

    bool IsOnSlope(const std::shared_ptr<Util::GameObject>& player,
                   float footOffset,
                   float tolerance) const;

    void SnapToSlope(const std::shared_ptr<Util::GameObject>& player,
                     float footOffset) const;

    bool IsLeftLowRightHigh() const;
    bool IsSolid() const { return m_IsSolid; }

    float GetSlideSpeed() const { return m_SlideSpeed; }
    float GetMoveFactor() const { return m_MoveFactor; }

    float GetStartX() const { return m_Start.x; }
    float GetEndX() const { return m_End.x; }
    float GetStartY() const { return m_Start.y; }

    std::shared_ptr<Util::GameObject> GetImage() const { return m_Image; }

private:
    std::shared_ptr<Util::GameObject> m_Image = nullptr;
    glm::vec2 m_Start{};
    glm::vec2 m_End{};

    float m_SlideSpeed = 0.0f;
    float m_MoveFactor = 1.0f;
    bool m_IsSolid = true;
};

#endif