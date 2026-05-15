#ifndef CHAIN_PLATFORM_H
#define CHAIN_PLATFORM_H

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <string>

class ChainPlatform {
public:
    ChainPlatform(
        const std::string& chainImagePath,
        const std::string& boardImagePath,
        const glm::vec2& chainPosition,
        const glm::vec2& boardPosition,
        const glm::vec2& chainScale,
        const glm::vec2& boardScale,
        float boardWidth,
        float boardHeight
    );

    void BeginFrame();
    void AddWeightAt(float worldX, float weight);
    void Update(float dt);

    std::shared_ptr<Util::GameObject> GetChainObject() const;
    std::shared_ptr<Util::GameObject> GetBoardObject() const;
    std::shared_ptr<Util::GameObject> GetObject() const;

    glm::vec2 GetPosition() const;
    glm::vec2 GetSize() const;

    float GetRotationDegree() const;
    void SetRotation(float degree);

    bool CheckCollisionWithPlayer(
    const glm::vec2& oldPlayerPos,
    glm::vec2& playerPos,
    const glm::vec2& playerSize,
    float& velocityY
    );

private:
    glm::vec2 WorldToLocal(const glm::vec2& worldPos) const;
    glm::vec2 LocalToWorld(const glm::vec2& localPos) const;

private:
    std::shared_ptr<Util::GameObject> m_ChainObject;
    std::shared_ptr<Util::GameObject> m_BoardObject;

    glm::vec2 m_ChainPosition;
    glm::vec2 m_BoardPosition;

    glm::vec2 m_ChainScale;
    glm::vec2 m_BoardScale;

    float m_BoardWidth;
    float m_BoardHeight;

    bool m_HasWeight = false;

    float m_RotationDegree = 0.0f;
    float m_AngularVelocity = 0.0f;

    // 360 版本：不要 MaxRotation
    float m_WeightForce = 0.20f;
    float m_RotationSpeed = 2.0f;
    float m_Damping = 0.985f;
    float m_StopThreshold = 0.003f;
};

#endif