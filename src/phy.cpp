#include "App.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include <cmath>
#include <algorithm>
#include <vector>
#include "slope.hpp"

bool App::IsColliding(const std::shared_ptr<Util::GameObject>& p,
                      const std::shared_ptr<Util::GameObject>& t) {
    if (!p || !t) return false;

    glm::vec2 pP = p->m_Transform.translation;
    glm::vec2 pS = p->GetScaledSize();
    glm::vec2 tP = t->m_Transform.translation;
    glm::vec2 tS = t->GetScaledSize();

    float pW = pS.x;
    float pH = pS.y;
    float tW = tS.x;
    float tH = tS.y;

    // 翻轉，目前用不到
    // if (m_Gears.size() >= 2) {
    //     if (p == m_Gears[1]) std::swap(pW, pH);
    //     if (t == m_Gears[1]) std::swap(tW, tH);
    // }

    return (pP.x - pW / 2.0f < tP.x + tW / 2.0f &&
            pP.x + pW / 2.0f > tP.x - tW / 2.0f &&
            pP.y - pH / 2.0f < tP.y + tH / 2.0f &&
            pP.y + pH / 2.0f > tP.y - tH / 2.0f);
}

void App::HandleMechanics(float iceDx, float fireDx, const Uint8* keys) {
    (void)keys;

    auto handleAdvancedPush = [&]() {
        const float PUSH_SPEED = 3.5f;
        const float SANDWICH_SPEED = 2.0f;

        if (iceDx != 0 && IsColliding(m_Ice, m_Box)) {
            float icePos = m_Ice->m_Transform.translation.x;
            float boxPos = m_Box->m_Transform.translation.x;
            bool isPushing = (icePos < boxPos && iceDx > 0) || (icePos > boxPos && iceDx < 0);
            if (isPushing) {
                float finalDx = (iceDx > 0 ? 1.0f : -1.0f);
                bool isSandwich = IsColliding(m_Box, m_Fire);
                if (isSandwich) {
                    finalDx *= SANDWICH_SPEED;
                    m_Fire->m_Transform.translation.x += finalDx;
                } else {
                    finalDx *= PUSH_SPEED;
                }
                m_Box->m_Transform.translation.x += finalDx;
                bool hitSomething = false;
                for (auto& s : m_Stones) {
                    if (IsColliding(m_Box, s)) {
                        hitSomething = true;
                        break;
                    }
                }
                
                // 改為遍歷向量 m_Gears
                for (auto& gear : m_Gears) {
                    if (IsColliding(m_Box, gear)) {
                        hitSomething = true;
                        break;
                    }
                }

                if (hitSomething) {
                    m_Box->m_Transform.translation.x -= finalDx;
                    if (isSandwich) m_Fire->m_Transform.translation.x -= finalDx;
                    iceDx = 0;
                } else {
                    iceDx = finalDx;
                    if (isSandwich) fireDx = finalDx;
                }
            }
        }

        if (fireDx != 0 && IsColliding(m_Fire, m_Box)) {
            float firePos = m_Fire->m_Transform.translation.x;
            float boxPos = m_Box->m_Transform.translation.x;
            bool isPushing = (firePos < boxPos && fireDx > 0) || (firePos > boxPos && fireDx < 0);
            if (isPushing) {
                float finalDx = (fireDx > 0 ? 1.0f : -1.0f);
                bool isSandwich = IsColliding(m_Box, m_Ice);
                if (isSandwich) {
                    finalDx *= SANDWICH_SPEED;
                    m_Ice->m_Transform.translation.x += finalDx;
                } else {
                    finalDx *= PUSH_SPEED;
                }
                m_Box->m_Transform.translation.x += finalDx;
                bool hitSomething = false;
                for (auto& s : m_Stones) {
                    if (IsColliding(m_Box, s)) {
                        hitSomething = true;
                        break;
                    }
                }

                // 改為遍歷向量 m_Gears
                for (auto& gear : m_Gears) {
                    if (IsColliding(m_Box, gear)) {
                        hitSomething = true;
                        break;
                    }
                }

                if (hitSomething) {
                    m_Box->m_Transform.translation.x -= finalDx;
                    if (isSandwich) m_Ice->m_Transform.translation.x -= finalDx;
                    fireDx = 0;
                } else {
                    fireDx = finalDx;
                    if (isSandwich) iceDx = finalDx;
                }
            }
        }
    };

    handleAdvancedPush();
    auto handleHorizontalObstacle = [&](std::shared_ptr<Util::GameObject> character, float& dx) {
        if (dx == 0) return;
        character->m_Transform.translation.x += dx;
        
        bool hitGear = false;
        // 改為遍歷向量 m_Gears
        for (auto& gear : m_Gears) {
            if (IsColliding(character, gear)) {
                hitGear = true;
                break;
            }
        }

        if (hitGear) {
            character->m_Transform.translation.x -= dx;
            dx = 0;
        } else {
            character->m_Transform.translation.x -= dx;
        }
    };
    handleHorizontalObstacle(m_Ice, iceDx);
    handleHorizontalObstacle(m_Fire, fireDx);

    m_IceVelocityY -= m_Gravity;
    m_FireVelocityY -= m_Gravity;
    m_Ice->m_Transform.translation.y += m_IceVelocityY;
    m_Fire->m_Transform.translation.y += m_FireVelocityY;

    bool iG = false, fG = false;
    std::vector<std::shared_ptr<Util::GameObject>> collisionGroup = m_Stones;
    if (m_Box) collisionGroup.push_back(m_Box);
    
    // 將所有向量中的 Gear 加入碰撞群組
    for (auto& gear : m_Gears) {
        collisionGroup.push_back(gear);
    }

    for (const auto& obj : collisionGroup) {
        float objW = obj->GetScaledSize().x;
        float objH = obj->GetScaledSize().y;

        // 處理向量中索引為 1 的 Gear2 旋轉邏輯
        // if (m_Gears.size() >= 2 && obj == m_Gears[1]) {
        //     std::swap(objW, objH);
        // }

        float objLeft   = obj->m_Transform.translation.x - (objW / 2.0f);
        float objRight  = obj->m_Transform.translation.x + (objW / 2.0f);
        float objTop    = obj->m_Transform.translation.y + (objH / 2.0f);
        float objBottom = obj->m_Transform.translation.y - (objH / 2.0f);

        if (IsColliding(m_Ice, obj)) {
            float iceHalfW = m_Ice->GetScaledSize().x / 2.0f;
            float iceHalfH = m_Ice->GetScaledSize().y / 2.0f;
            float iceLeft = m_Ice->m_Transform.translation.x - iceHalfW;
            float iceRight = m_Ice->m_Transform.translation.x + iceHalfW;
            float iceTop = m_Ice->m_Transform.translation.y + iceHalfH;
            float iceBottom = m_Ice->m_Transform.translation.y - iceHalfH;

            float overlapLeft = iceRight - objLeft;
            float overlapRight = objRight - iceLeft;
            float overlapTop = iceTop - objBottom;
            float overlapBottom = objTop - iceBottom;

            float minOverlapX = std::min(overlapLeft, overlapRight);
            float minOverlapY = std::min(overlapTop, overlapBottom);

            if (minOverlapX < minOverlapY) {
                if (overlapLeft < overlapRight) {
                    m_Ice->m_Transform.translation.x = objLeft - iceHalfW;
                } else {
                    m_Ice->m_Transform.translation.x = objRight + iceHalfW;
                }
            } else {
                if (m_IceVelocityY <= 0 && m_Ice->m_Transform.translation.y > obj->m_Transform.translation.y) {
                    m_IceVelocityY = 0;
                    m_Ice->m_Transform.translation.y = objTop + iceHalfH;
                    iG = true;
                } else if (m_IceVelocityY > 0 && m_Ice->m_Transform.translation.y < obj->m_Transform.translation.y) {
                    m_IceVelocityY = 0;
                    m_Ice->m_Transform.translation.y = objBottom - iceHalfH;
                }
            }
        }
        if (IsColliding(m_Fire, obj)) {
            float fireHalfW = m_Fire->GetScaledSize().x / 2.0f;
            float fireHalfH = m_Fire->GetScaledSize().y / 2.0f;
            float fireLeft = m_Fire->m_Transform.translation.x - fireHalfW;
            float fireRight = m_Fire->m_Transform.translation.x + fireHalfW;
            float fireTop = m_Fire->m_Transform.translation.y + fireHalfH;
            float fireBottom = m_Fire->m_Transform.translation.y - fireHalfH;

            float overlapLeft = fireRight - objLeft;
            float overlapRight = objRight - fireLeft;
            float overlapTop = fireTop - objBottom;
            float overlapBottom = objTop - fireBottom;

            float minOverlapX = std::min(overlapLeft, overlapRight);
            float minOverlapY = std::min(overlapTop, overlapBottom);

            if (minOverlapX < minOverlapY) {
                if (overlapLeft < overlapRight) {
                    m_Fire->m_Transform.translation.x = objLeft - fireHalfW;
                } else {
                    m_Fire->m_Transform.translation.x = objRight + fireHalfW;
                }
            } else {
                if (m_FireVelocityY <= 0 && m_Fire->m_Transform.translation.y > obj->m_Transform.translation.y) {
                    m_FireVelocityY = 0;
                    m_Fire->m_Transform.translation.y = objTop + fireHalfH;
                    fG = true;
                } else if (m_FireVelocityY > 0 && m_Fire->m_Transform.translation.y < obj->m_Transform.translation.y) {
                    m_FireVelocityY = 0;
                    m_Fire->m_Transform.translation.y = objBottom - fireHalfH;
                }
            }
        }
    }

    m_IceOnGround = iG; 
    m_FireOnGround = fG;

    ApplySlopeToPlayer(m_Ice, m_IceVelocityY, m_IceOnGround, iceDx);
    ApplySlopeToPlayer(m_Fire, m_FireVelocityY, m_FireOnGround, fireDx);

    // 箱子垂直碰撞邏輯
    std::vector<std::shared_ptr<Util::GameObject>> boxCollisionGroup = m_Stones;
    for (auto& gear : m_Gears) {
        boxCollisionGroup.push_back(gear);
    }

    if (m_Box) {
        m_BoxOnGround = false;
        m_BoxVelocityY -= m_Gravity;
        m_Box->m_Transform.translation.y += m_BoxVelocityY;
        for (const auto &obj: boxCollisionGroup) {
            if (!obj) continue;
            if (IsColliding(m_Box, obj)) {
                float objTop = obj->m_Transform.translation.y + obj->GetScaledSize().y / 2.0f;
                float objBottom = obj->m_Transform.translation.y - obj->GetScaledSize().y / 2.0f;
                float boxHalfH = m_Box->GetScaledSize().y / 2.0f;
                if (m_BoxVelocityY <= 0.0f && m_Box->m_Transform.translation.y > obj->m_Transform.translation.y) {
                    m_Box->m_Transform.translation.y = objTop + boxHalfH;
                    m_BoxVelocityY = 0.0f;
                    m_BoxOnGround = true;
                } else if (m_BoxVelocityY > 0.0f && m_Box->m_Transform.translation.y < obj->m_Transform.translation.y) {
                    m_Box->m_Transform.translation.y = objBottom - boxHalfH;
                    m_BoxVelocityY = 0.0f;
                }
            }
        }
    }
    auto isStandingOnTop = [&](std::shared_ptr<Util::GameObject> character, std::shared_ptr<Util::GameObject> platform) {
        if (!character || !platform) return false;
        float charHalfW = character->GetScaledSize().x / 2.0f;
        float charHalfH = character->GetScaledSize().y / 2.0f;
        float platHalfW = platform->GetScaledSize().x / 2.0f;
        float platHalfH = platform->GetScaledSize().y / 2.0f;

        // 針對向量中的第二個 Gear 進行寬高交換邏輯
        // if (m_Gears.size() >= 2 && platform == m_Gears[1]) {
        //     std::swap(platHalfW, platHalfH);
        // }

        float charLeft = character->m_Transform.translation.x - charHalfW;
        float charRight = character->m_Transform.translation.x + charHalfW;
        float charBottom = character->m_Transform.translation.y - charHalfH;

        float platLeft = platform->m_Transform.translation.x - platHalfW;
        float platRight = platform->m_Transform.translation.x + platHalfW;
        float platTop = platform->m_Transform.translation.y + platHalfH;

        return (charRight > platLeft) && (charLeft < platRight) && std::abs(charBottom - platTop) < 5.0f;
    };

    // 向量化機關邏輯：按鈕 (Buttons)
    bool anyButtonPressed = false;
    for (size_t i = 0; i < m_Buttons.size(); ++i) {
        bool pressed = IsColliding(m_Ice, m_Buttons[i]) || 
                       IsColliding(m_Fire, m_Buttons[i]) || 
                       (m_Box && IsColliding(m_Box, m_Buttons[i]));
        m_Buttons[i]->SetVisible(!pressed);
        if (pressed) anyButtonPressed = true;
    }

    // 向量化機關邏輯：拉桿 (Switches)
    auto handleSwitch = [&](std::shared_ptr<Util::GameObject> character, float dx, bool isIce) {
        for (size_t i = 0; i < m_Switches.size(); ++i) {
            if (IsColliding(character, m_Switches[i])) {
                float charX = character->m_Transform.translation.x;
                float swX = m_Switches[i]->m_Transform.translation.x;
                bool pushingRight = isIce ? keys[SDL_SCANCODE_D] : keys[SDL_SCANCODE_RIGHT];
                bool pushingLeft = isIce ? keys[SDL_SCANCODE_A] : keys[SDL_SCANCODE_LEFT];

                if (charX < swX && dx > 0 && pushingRight && m_SwitchStates[i]) {
                    m_SwitchStates[i] = false;
                    m_Switches[i]->SetDrawable(std::make_shared<Util::Image>(PIC_PATH + "switch1_1.png"));
                } else if (charX > swX && dx < 0 && pushingLeft && !m_SwitchStates[i]) {
                    m_SwitchStates[i] = true;
                    m_Switches[i]->SetDrawable(std::make_shared<Util::Image>(PIC_PATH + "switch1_2.png"));
                }
            }
        }
    };
    handleSwitch(m_Ice, iceDx, true);
    handleSwitch(m_Fire, fireDx, false);
    // 8. 處理 Gears 位移與角色同步 (向量化遍歷)
    for (size_t i = 0; i < m_Gears.size(); ++i) {
        if (!m_Gears[i]) continue;

        glm::vec2 oldPos = m_Gears[i]->m_Transform.translation;
        bool iceOn = isStandingOnTop(m_Ice, m_Gears[i]);
        bool fireOn = isStandingOnTop(m_Fire, m_Gears[i]);

        // 邏輯保持：i=0 受按鈕控制, i=1 受拉桿控制
        float targetY = m_GearOriginalPositions[i].y;
        if (i == 0) {
            targetY = anyButtonPressed ? (m_GearOriginalPositions[0].y - 75.0f) : m_GearOriginalPositions[0].y;
        } else if (i == 1) {
            targetY = (!m_SwitchStates.empty() && m_SwitchStates[0]) ? (m_GearOriginalPositions[1].y - 75.0f) : m_GearOriginalPositions[1].y;
        }

        float speed = 2.0f;
        if (m_Gears[i]->m_Transform.translation.y < targetY) {
            m_Gears[i]->m_Transform.translation.y = std::min(m_Gears[i]->m_Transform.translation.y + speed, targetY);
        } else if (m_Gears[i]->m_Transform.translation.y > targetY) {
            m_Gears[i]->m_Transform.translation.y = std::max(m_Gears[i]->m_Transform.translation.y - speed, targetY);
        }

        glm::vec2 delta = m_Gears[i]->m_Transform.translation - oldPos;
        if (delta.x != 0.0f || delta.y != 0.0f) {
            if (iceOn) {
                m_Ice->m_Transform.translation += delta;
                m_IceVelocityY = 0.0f;
                m_IceOnGround = true;
            }
            if (fireOn) {
                m_Fire->m_Transform.translation += delta;
                m_FireVelocityY = 0.0f;
                m_FireOnGround = true;
            }
        }
    }

    // 9. 門的動畫處理與關卡切換
    m_IceDoorOpening = IsColliding(m_Ice, m_IceDoor);
    m_FireDoorOpening = IsColliding(m_Fire, m_FireDoor);
    m_DoorAnimCounter++;

    if (m_DoorAnimCounter >= m_DoorAnimSpeed) {
        m_DoorAnimCounter = 0;
        if (m_IceDoorOpening) {
            if (m_IceDoorFrameIndex < (int)m_IceDoorFrames.size() - 1) m_IceDoorFrameIndex++;
        } else {
            if (m_IceDoorFrameIndex > 0) m_IceDoorFrameIndex--;
        }
        m_IceDoor->SetDrawable(std::make_shared<Util::Image>(m_IceDoorFrames[m_IceDoorFrameIndex]));

        if (m_FireDoorOpening) {
            if (m_FireDoorFrameIndex < (int)m_FireDoorFrames.size() - 1) m_FireDoorFrameIndex++;
        } else {
            if (m_FireDoorFrameIndex > 0) m_FireDoorFrameIndex--;
        }
        m_FireDoor->SetDrawable(std::make_shared<Util::Image>(m_FireDoorFrames[m_FireDoorFrameIndex]));
    }

    // 檢查通關條件
    if (m_IceDoorFrameIndex == (int)m_IceDoorFrames.size() - 1 &&
        m_FireDoorFrameIndex == (int)m_FireDoorFrames.size() - 1 &&
        IsColliding(m_Ice, m_IceDoor) && IsColliding(m_Fire, m_FireDoor)) {
        LoadLevel(m_CurrentLevelNum + 1);
        return;
    }

    // 10. 陷阱與死亡判定
    bool iceDead = false, fireDead = false;
    for (const auto& trap : m_Traps) {
        if (IsColliding(m_Ice, trap)) { iceDead = true; break; }
    }
    if (!iceDead) {
        for (const auto& trap : m_IceTraps) {
            if (IsColliding(m_Ice, trap)) { iceDead = true; break; }
        }
    }

    for (const auto& trap : m_Traps) {
        if (IsColliding(m_Fire, trap)) { fireDead = true; break; }
    }
    if (!fireDead) {
        for (const auto& trap : m_FireTraps) {
            if (IsColliding(m_Fire, trap)) { fireDead = true; break; }
        }
    }

    if (iceDead || fireDead) {
        m_CurrentState = State::DEAD;
        m_DeadScreen->SetVisible(true);
    }
}