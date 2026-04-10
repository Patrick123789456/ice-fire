#include "Slope.hpp"
#include <algorithm>
#include <cmath>
#include "Util/Image.hpp"
#include "App.hpp"

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

void App::AddSlope(const std::string& imagePath, const glm::vec2& imagePos,
                  const glm::vec2& imageScale, const glm::vec2& localStart,
                  const glm::vec2& localEnd, float slideSpeed,
                  float moveFactor, bool isSolid, float zIndex) {

    auto image = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(imagePath), zIndex
    );

    image->m_Transform.translation = imagePos;
    image->m_Transform.scale = imageScale;
    m_Root->AddChild(image); // 現在沒問題了，因為 App 看得到 m_Root

    glm::vec2 start = imagePos + glm::vec2(localStart.x * imageScale.x,
                                           localStart.y * imageScale.y);
    glm::vec2 end = imagePos + glm::vec2(localEnd.x * imageScale.x,
                                         localEnd.y * imageScale.y);

    m_Slopes.emplace_back(image, start, end, slideSpeed, moveFactor, isSolid);
}

void App::ApplySlopeToPlayer(const std::shared_ptr<Util::GameObject>& player,
                             float& velocityY, bool& onGround, float& dx) {
    float currentX = player->m_Transform.translation.x;
    float currentY = player->m_Transform.translation.y;
    float footY = currentY - m_FootOffset;

    for (const auto& slope : m_Slopes) {
        if (!slope.IsSolid()) continue;

        float minX = std::min(slope.GetStartX(), slope.GetEndX());
        float maxX = std::max(slope.GetStartX(), slope.GetEndX());

        float currentSurfaceY = slope.GetSurfaceY(currentX);
        bool alreadyOnSlope = currentX >= minX && currentX <= maxX &&
                              std::abs(footY - currentSurfaceY) <= m_SlopeTolerance;

        float moveDx = dx;
        if (std::abs(moveDx) >= 0.001f) {
            moveDx *= slope.GetMoveFactor();
        } else if (alreadyOnSlope) {
            moveDx = slope.IsLeftLowRightHigh() ? -slope.GetSlideSpeed() : slope.GetSlideSpeed();
        }

        float nextX = currentX + moveDx;
        if (nextX < minX || nextX > maxX) {
            if (alreadyOnSlope) {
                player->m_Transform.translation.x = nextX;
                dx = 0; // 離開斜坡時停止水平位移避免瞬間噴射
                return;
            }
            continue;
        }

        float nextSurfaceY = slope.GetSurfaceY(nextX);
        float nextFootY = footY + velocityY;

        bool crossedSlope = footY >= nextSurfaceY && nextFootY <= nextSurfaceY;
        bool canSnapToSlope = nextFootY >= nextSurfaceY - m_SlopeTolerance &&
                              nextFootY <= nextSurfaceY + m_SlopeSnapHeight;

        if (alreadyOnSlope || canSnapToSlope || crossedSlope) {
            onGround = true;
            velocityY = 0.0f;
            player->m_Transform.translation.x = nextX;
            player->m_Transform.translation.y = nextSurfaceY + m_FootOffset;
            dx = 0; // 成功吸附，消耗掉這次的位移
            return;
        }
    }
    // 如果沒在斜坡上，執行一般位移
    player->m_Transform.translation.x += dx;
}