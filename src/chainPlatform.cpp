#include "chainPlatform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

ChainPlatform::ChainPlatform(
    const std::string& chainImagePath,
    const std::string& boardImagePath,
    const glm::vec2& chainPosition,
    const glm::vec2& boardPosition,
    const glm::vec2& chainScale,
    const glm::vec2& boardScale,
    float boardWidth,
    float boardHeight
) {
    m_ChainObject = std::make_shared<Util::GameObject>(
        std::make_unique<Util::Image>(chainImagePath),
        9
    );

    m_BoardObject = std::make_shared<Util::GameObject>(
        std::make_unique<Util::Image>(boardImagePath),
        10
    );

    m_ChainPosition = chainPosition;
    m_BoardPosition = boardPosition;

    m_ChainScale = chainScale;
    m_BoardScale = boardScale;

    m_BoardWidth = boardWidth;
    m_BoardHeight = boardHeight;

    m_ChainObject->m_Transform.translation = m_ChainPosition;
    m_ChainObject->m_Transform.scale = m_ChainScale;
    m_ChainObject->m_Transform.rotation = 0.0f;

    m_BoardObject->m_Transform.translation = m_BoardPosition;
    m_BoardObject->m_Transform.scale = m_BoardScale;
    m_BoardObject->m_Transform.rotation = 0.0f;
}

void ChainPlatform::BeginFrame() {
    m_HasWeight = false;
}

void ChainPlatform::AddWeightAt(float worldX, float weight) {
    float dx = worldX - m_BoardPosition.x;
    float halfW = m_BoardWidth * m_BoardScale.x / 2.0f;

    if (std::abs(dx) > halfW) return;

    float side = dx / halfW;

    if (std::abs(side) < 0.05f) {
        side = side >= 0.0f ? 0.05f : -0.05f;
    }

    m_HasWeight = true;

    // 站右邊就順時針轉，站左邊就逆時針轉
    m_AngularVelocity -= side * weight * m_WeightForce;
}

void ChainPlatform::Update(float dt) {
    if (!m_HasWeight) {
        m_AngularVelocity *= m_Damping;

        if (std::abs(m_AngularVelocity) < m_StopThreshold) {
            m_AngularVelocity = 0.0f;
        }
    }

    m_RotationDegree += m_AngularVelocity * m_RotationSpeed * dt;

    // 讓角度保持在 0 ~ 360，避免數字無限變大
    while (m_RotationDegree >= 360.0f) {
        m_RotationDegree -= 360.0f;
    }

    while (m_RotationDegree < 0.0f) {
        m_RotationDegree += 360.0f;
    }

    m_BoardObject->m_Transform.rotation = glm::radians(m_RotationDegree);

    // 鐵鍊固定不動
    m_ChainObject->m_Transform.rotation = 0.0f;
}

glm::vec2 ChainPlatform::WorldToLocal(const glm::vec2& worldPos) const {
    glm::vec2 p = worldPos - m_BoardPosition;

    float rad = -glm::radians(m_RotationDegree);

    float c = std::cos(rad);
    float s = std::sin(rad);

    return {
        p.x * c - p.y * s,
        p.x * s + p.y * c
    };
}

glm::vec2 ChainPlatform::LocalToWorld(const glm::vec2& localPos) const {
    float rad = glm::radians(m_RotationDegree);

    float c = std::cos(rad);
    float s = std::sin(rad);

    return {
        m_BoardPosition.x + localPos.x * c - localPos.y * s,
        m_BoardPosition.y + localPos.x * s + localPos.y * c
    };
}

bool ChainPlatform::CheckCollisionWithPlayer(
    const glm::vec2& oldPlayerPos,
    glm::vec2& playerPos,
    const glm::vec2& playerSize,
    float& velocityY
) {
    (void)oldPlayerPos;

    float halfPlayerW = playerSize.x / 2.0f;
    float halfPlayerH = playerSize.y / 2.0f;

    float halfBoardW = m_BoardWidth * m_BoardScale.x / 2.0f;
    float halfBoardH = m_BoardHeight * m_BoardScale.y / 2.0f;

    float rad = glm::radians(m_RotationDegree);
    float c = std::cos(rad);
    float s = std::sin(rad);

    // 平台的兩個方向軸
    glm::vec2 boardRight = { c, s };
    glm::vec2 boardUp = { -s, c };

    // 角色四個角，世界座標
    glm::vec2 playerCorners[4] = {
        { playerPos.x - halfPlayerW, playerPos.y - halfPlayerH },
        { playerPos.x + halfPlayerW, playerPos.y - halfPlayerH },
        { playerPos.x + halfPlayerW, playerPos.y + halfPlayerH },
        { playerPos.x - halfPlayerW, playerPos.y + halfPlayerH }
    };

    // 平台四個角，世界座標
    glm::vec2 boardCorners[4] = {
        m_BoardPosition + (-halfBoardW * boardRight) + (-halfBoardH * boardUp),
        m_BoardPosition + ( halfBoardW * boardRight) + (-halfBoardH * boardUp),
        m_BoardPosition + ( halfBoardW * boardRight) + ( halfBoardH * boardUp),
        m_BoardPosition + (-halfBoardW * boardRight) + ( halfBoardH * boardUp)
    };

    auto project = [](const glm::vec2 corners[4], const glm::vec2& axis,
                      float& minValue, float& maxValue) {
        minValue = glm::dot(corners[0], axis);
        maxValue = minValue;

        for (int i = 1; i < 4; i++) {
            float p = glm::dot(corners[i], axis);

            if (p < minValue) minValue = p;
            if (p > maxValue) maxValue = p;
        }
    };

    glm::vec2 axes[4] = {
        { 1.0f, 0.0f },  // 角色 X 軸
        { 0.0f, 1.0f },  // 角色 Y 軸
        boardRight,      // 平台旋轉後的 X 軸
        boardUp          // 平台旋轉後的 Y 軸
    };

    float minOverlap = 999999.0f;
    glm::vec2 smallestAxis = { 0.0f, 1.0f };

    for (int i = 0; i < 4; i++) {
        glm::vec2 axis = glm::normalize(axes[i]);

        float pMin, pMax;
        float bMin, bMax;

        project(playerCorners, axis, pMin, pMax);
        project(boardCorners, axis, bMin, bMax);

        // 沒重疊，代表沒有碰撞
        if (pMax < bMin || bMax < pMin) {
            return false;
        }

        float overlap1 = pMax - bMin;
        float overlap2 = bMax - pMin;
        float overlap = std::min(overlap1, overlap2);

        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    // 推出的方向：從平台中心推向角色中心
    glm::vec2 dir = playerPos - m_BoardPosition;

    if (glm::dot(dir, smallestAxis) < 0.0f) {
        smallestAxis = -smallestAxis;
    }

    // 只推剛好離開，不要加太多，不然縫隙會變大
    glm::vec2 push = smallestAxis * (minOverlap + 0.1f);

    playerPos += push;

    // 被往上推，代表角色站在平台上
    bool standOnPlatform = push.y > 0.1f;

    if (standOnPlatform) {
        velocityY = 0.0f;

        // 給平台重量
        AddWeightAt(playerPos.x, 12.0f);

        return true;
    }

    // 頭撞到平台下面
    if (push.y < -0.1f && velocityY > 0.0f) {
        velocityY = 0.0f;
    }

    return false;
}

std::shared_ptr<Util::GameObject> ChainPlatform::GetChainObject() const {
    return m_ChainObject;
}

std::shared_ptr<Util::GameObject> ChainPlatform::GetBoardObject() const {
    return m_BoardObject;
}

std::shared_ptr<Util::GameObject> ChainPlatform::GetObject() const {
    return m_BoardObject;
}

glm::vec2 ChainPlatform::GetPosition() const {
    return m_BoardPosition;
}

glm::vec2 ChainPlatform::GetSize() const {
    return {
        m_BoardWidth * m_BoardScale.x,
        m_BoardHeight * m_BoardScale.y
    };
}

float ChainPlatform::GetRotationDegree() const {
    return m_RotationDegree;
}

void ChainPlatform::SetRotation(float degree) {
    m_RotationDegree = degree;
    m_AngularVelocity = 0.0f;

    m_BoardObject->m_Transform.rotation = glm::radians(m_RotationDegree);
    m_ChainObject->m_Transform.rotation = 0.0f;
}