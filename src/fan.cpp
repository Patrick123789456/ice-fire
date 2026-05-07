#include "fan.h"

Fan::Fan(const std::vector<std::string>& fanFrames,
         const std::vector<std::string>& windFrames)
    : m_FanFrames(fanFrames),
      m_WindFrames(windFrames) {

    m_FanObject = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(m_FanFrames[0]),
        3.0f
    );

    m_WindObject = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(m_WindFrames[0]),
        2.0f
    );

    m_FanObject->SetVisible(false);
    m_WindObject->SetVisible(false);
}

std::shared_ptr<Util::GameObject> Fan::GetFanObject() const {
    return m_FanObject;
}

std::shared_ptr<Util::GameObject> Fan::GetWindObject() const {
    return m_WindObject;
}

void Fan::SetActive(bool active) {
    m_Active = active;

    if (m_FanObject) {
        m_FanObject->SetVisible(active);
    }

    if (m_WindObject) {
        m_WindObject->SetVisible(active);
    }
}

bool Fan::IsActive() const {
    return m_Active;
}

void Fan::SetPosition(glm::vec2 fanPos, glm::vec2 windOffset) {
    if (!m_FanObject || !m_WindObject) return;

    m_FanObject->m_Transform.translation = fanPos;
    m_WindObject->m_Transform.translation = fanPos + windOffset;
}

void Fan::SetScale(glm::vec2 fanScale, glm::vec2 windScale) {
    if (!m_FanObject || !m_WindObject) return;

    m_FanObject->m_Transform.scale = fanScale;
    m_WindObject->m_Transform.scale = windScale;
}

void Fan::UpdateAnimation() {
    if (!m_Active) return;

    if (m_FanObject && !m_FanFrames.empty()) {
        m_FanAnimCounter++;

        if (m_FanAnimCounter >= m_FanAnimSpeed) {
            m_FanAnimCounter = 0;

            m_FanFrameIndex++;
            if (m_FanFrameIndex >= static_cast<int>(m_FanFrames.size())) {
                m_FanFrameIndex = 0;
            }

            m_FanObject->SetDrawable(
                std::make_shared<Util::Image>(m_FanFrames[m_FanFrameIndex])
            );
        }
    }

    if (m_WindObject && !m_WindFrames.empty()) {
        m_WindAnimCounter++;

        if (m_WindAnimCounter >= m_WindAnimSpeed) {
            m_WindAnimCounter = 0;

            m_WindFrameIndex++;
            if (m_WindFrameIndex >= static_cast<int>(m_WindFrames.size())) {
                m_WindFrameIndex = 0;
            }

            m_WindObject->SetDrawable(
                std::make_shared<Util::Image>(m_WindFrames[m_WindFrameIndex])
            );
        }
    }
}

bool Fan::IsInWindArea(const std::shared_ptr<Util::GameObject>& player) const {
    if (!player || !m_FanObject) return false;

    glm::vec2 p = player->m_Transform.translation;
    glm::vec2 f = m_FanObject->m_Transform.translation;

    float left = f.x - m_WindWidth / 2.0f;
    float right = f.x + m_WindWidth / 2.0f;
    float bottom = f.y;
    float top = f.y + m_WindHeight;

    return p.x >= left &&
           p.x <= right &&
           p.y >= bottom &&
           p.y <= top;
}

void Fan::ApplyWind(
    const std::shared_ptr<Util::GameObject>& player,
    float& velocityY,
    bool& onGround
) {
    if (!m_Active) return;
    if (!player) return;

    if (IsInWindArea(player)) {
        // 讓角色離開地面狀態，不然會一直被地板碰撞拉回去
        onGround = false;

        // 直接給向上的速度
        // 如果現在往下掉，就先抵消掉
        if (velocityY < m_WindPower) {
            velocityY = m_WindPower;
        }

        // 稍微往上推一點，避免一直卡在地板接觸判定
        player->m_Transform.translation.y += 1.0f;
    }
}