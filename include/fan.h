#ifndef FAN_H
#define FAN_H

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

class Fan {
public:
    Fan(const std::vector<std::string>& fanFrames,
        const std::vector<std::string>& windFrames);

    std::shared_ptr<Util::GameObject> GetFanObject() const;
    std::shared_ptr<Util::GameObject> GetWindObject() const;

    void SetActive(bool active);
    bool IsActive() const;

    void SetPosition(glm::vec2 fanPos, glm::vec2 windOffset);
    void SetScale(glm::vec2 fanScale, glm::vec2 windScale);

    void UpdateAnimation();
    void ApplyWind(
        const std::shared_ptr<Util::GameObject>& player,
        float& velocityY,
        bool& onGround
    );

private:
    bool IsInWindArea(const std::shared_ptr<Util::GameObject>& player) const;

private:
    std::shared_ptr<Util::GameObject> m_FanObject;
    std::shared_ptr<Util::GameObject> m_WindObject;

    std::vector<std::string> m_FanFrames;
    std::vector<std::string> m_WindFrames;

    int m_FanFrameIndex = 0;
    int m_WindFrameIndex = 0;

    int m_FanAnimCounter = 0;
    int m_WindAnimCounter = 0;

    int m_FanAnimSpeed = 4;
    int m_WindAnimSpeed = 3;

    bool m_Active = false;

    float m_WindWidth = 90.0f;
    float m_WindHeight = 230.0f;
    float m_WindPower = 3.0f;
};

#endif