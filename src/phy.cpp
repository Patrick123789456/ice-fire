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

    // gear2 圖轉了 90 度，碰撞箱寬高也要交換
    if (p == m_Gear2) std::swap(pW, pH);
    if (t == m_Gear2) std::swap(tW, tH);

    return (pP.x - pW / 2.0f < tP.x + tW / 2.0f &&
            pP.x + pW / 2.0f > tP.x - tW / 2.0f &&
            pP.y - pH / 2.0f < tP.y + tH / 2.0f &&
            pP.y + pH / 2.0f > tP.y - tH / 2.0f);
}

void App::HandleMechanics(float iceDx, float fireDx, const Uint8* keys) {
    (void)keys; // 消除 unused parameter 'keys' 警告

    // 2. 處理水平推箱子
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
                if (isSandwich) { finalDx *= SANDWICH_SPEED; m_Fire->m_Transform.translation.x += finalDx; }
                else { finalDx *= PUSH_SPEED; }
                m_Box->m_Transform.translation.x += finalDx;
                bool hitSomething = false;
                for (auto& s : m_Stones) { if (IsColliding(m_Box, s)) { hitSomething = true; break; } }
                if (IsColliding(m_Box, m_Gear) || IsColliding(m_Box, m_Gear2)) hitSomething = true;
                if (hitSomething) {
                    m_Box->m_Transform.translation.x -= finalDx;
                    if (isSandwich) m_Fire->m_Transform.translation.x -= finalDx;
                    iceDx = 0;
                } else { iceDx = finalDx; if (isSandwich) fireDx = finalDx; }
            }
        }

        if (fireDx != 0 && IsColliding(m_Fire, m_Box)) {
            float firePos = m_Fire->m_Transform.translation.x;
            float boxPos = m_Box->m_Transform.translation.x;
            bool isPushing = (firePos < boxPos && fireDx > 0) || (firePos > boxPos && fireDx < 0);
            if (isPushing) {
                float finalDx = (fireDx > 0 ? 1.0f : -1.0f);
                bool isSandwich = IsColliding(m_Box, m_Ice);
                if (isSandwich) { finalDx *= SANDWICH_SPEED; m_Ice->m_Transform.translation.x += finalDx; }
                else { finalDx *= PUSH_SPEED; }
                m_Box->m_Transform.translation.x += finalDx;
                bool hitSomething = false;
                for (auto& s : m_Stones) { if (IsColliding(m_Box, s)) { hitSomething = true; break; } }
                if (IsColliding(m_Box, m_Gear) || IsColliding(m_Box, m_Gear2)) hitSomething = true;
                if (hitSomething) {
                    m_Box->m_Transform.translation.x -= finalDx;
                    if (isSandwich) m_Ice->m_Transform.translation.x -= finalDx;
                    fireDx = 0;
                } else { fireDx = finalDx; if (isSandwich) iceDx = finalDx; }
            }
        }
    };

    handleAdvancedPush();

    // 3. 處理與機關(Gear)的水平阻擋
    auto handleHorizontalObstacle = [&](std::shared_ptr<Util::GameObject> character, float& dx) {
        if (dx == 0) return;
        character->m_Transform.translation.x += dx;
        if (IsColliding(character, m_Gear)|| IsColliding(character, m_Gear2)) { character->m_Transform.translation.x -= dx; dx = 0; }
        else { character->m_Transform.translation.x -= dx; }
    };
    handleHorizontalObstacle(m_Ice, iceDx);
    handleHorizontalObstacle(m_Fire, fireDx);

    /* //擴充水平阻擋
    auto handleHorizontalObstacleAll = [&](std::shared_ptr<Util::GameObject> character, float& dx) {
        if (dx == 0) return;
        character->m_Transform.translation.x += dx;
        if (IsColliding(character, m_Gear) || IsColliding(character, m_Gear2)) { // 加入 Gear2
            character->m_Transform.translation.x -= dx;
            dx = 0;
        } else {
            character->m_Transform.translation.x -= dx;
        }
    };
*/ //目前箱子不會撞到gear，所以先不加入gear2的判定(其實1也用不到)

    // 執行最後位移
    m_IceVelocityY -= m_Gravity;
    m_FireVelocityY -= m_Gravity;
    m_Ice->m_Transform.translation.y += m_IceVelocityY;
    m_Fire->m_Transform.translation.y += m_FireVelocityY;

    // 4. 地板與頭部碰撞判定 (包含機關阻擋)
    bool iG = false, fG = false;
    std::vector<std::shared_ptr<Util::GameObject>> collisionGroup = m_Stones;
    if (m_Box) collisionGroup.push_back(m_Box);
    if (m_Gear) collisionGroup.push_back(m_Gear);
    if (m_Gear2) collisionGroup.push_back(m_Gear2);

    for (const auto& obj : collisionGroup) {
        float objW = obj->GetScaledSize().x;
        float objH = obj->GetScaledSize().y;

        if (obj == m_Gear2) {
            std::swap(objW, objH);
        }

        float objLeft   = obj->m_Transform.translation.x - (objW / 2.0f);
        float objRight  = obj->m_Transform.translation.x + (objW / 2.0f);
        float objTop    = obj->m_Transform.translation.y + (objH / 2.0f);
        float objBottom = obj->m_Transform.translation.y - (objH / 2.0f);

    // ===== Ice =====
    if (IsColliding(m_Ice, obj)) {
        float iceHalfW = m_Ice->GetScaledSize().x / 2.0f;
        float iceHalfH = m_Ice->GetScaledSize().y / 2.0f;

        float iceLeft   = m_Ice->m_Transform.translation.x - iceHalfW;
        float iceRight  = m_Ice->m_Transform.translation.x + iceHalfW;
        float iceTop    = m_Ice->m_Transform.translation.y + iceHalfH;
        float iceBottom = m_Ice->m_Transform.translation.y - iceHalfH;

        float overlapLeft   = iceRight - objLeft;     // 冰角色從左撞過來
        float overlapRight  = objRight - iceLeft;     // 冰角色從右撞過來
        float overlapTop    = iceTop - objBottom;     // 冰角色從下往上頂
        float overlapBottom = objTop - iceBottom;     // 冰角色從上往下踩

        float minOverlapX = std::min(overlapLeft, overlapRight);
        float minOverlapY = std::min(overlapTop, overlapBottom);

        if (minOverlapX < minOverlapY) {
            // 左右碰撞
            if (overlapLeft < overlapRight) {
                m_Ice->m_Transform.translation.x = objLeft - iceHalfW;
            } else {
                m_Ice->m_Transform.translation.x = objRight + iceHalfW;
            }
        } else {
            // 上下碰撞
            if (m_IceVelocityY <= 0 && m_Ice->m_Transform.translation.y > obj->m_Transform.translation.y) {
                m_IceVelocityY = 0;
                m_Ice->m_Transform.translation.y = objTop + iceHalfH;
                iG = true;
            }
            else if (m_IceVelocityY > 0 && m_Ice->m_Transform.translation.y < obj->m_Transform.translation.y) {
                m_IceVelocityY = 0;
                m_Ice->m_Transform.translation.y = objBottom - iceHalfH;
            }
        }
    }

    // ===== Fire =====
    if (IsColliding(m_Fire, obj)) {
        float fireHalfW = m_Fire->GetScaledSize().x / 2.0f;
        float fireHalfH = m_Fire->GetScaledSize().y / 2.0f;

        float fireLeft   = m_Fire->m_Transform.translation.x - fireHalfW;
        float fireRight  = m_Fire->m_Transform.translation.x + fireHalfW;
        float fireTop    = m_Fire->m_Transform.translation.y + fireHalfH;
        float fireBottom = m_Fire->m_Transform.translation.y - fireHalfH;

        float overlapLeft   = fireRight - objLeft;
        float overlapRight  = objRight - fireLeft;
        float overlapTop    = fireTop - objBottom;
        float overlapBottom = objTop - fireBottom;

        float minOverlapX = std::min(overlapLeft, overlapRight);
        float minOverlapY = std::min(overlapTop, overlapBottom);

        if (minOverlapX < minOverlapY) {
            // 左右碰撞
            if (overlapLeft < overlapRight) {
                m_Fire->m_Transform.translation.x = objLeft - fireHalfW;
            } else {
                m_Fire->m_Transform.translation.x = objRight + fireHalfW;
            }
        } else {
            // 上下碰撞
            if (m_FireVelocityY <= 0 && m_Fire->m_Transform.translation.y > obj->m_Transform.translation.y) {
                m_FireVelocityY = 0;
                m_Fire->m_Transform.translation.y = objTop + fireHalfH;
                fG = true;
            }
            else if (m_FireVelocityY > 0 && m_Fire->m_Transform.translation.y < obj->m_Transform.translation.y) {
                m_FireVelocityY = 0;
                m_Fire->m_Transform.translation.y = objBottom - fireHalfH;
            }
        }
    }
}
    m_IceOnGround = iG; m_FireOnGround = fG;
    ApplySlopeToPlayer(m_Ice, m_IceVelocityY, m_IceOnGround, iceDx);
    ApplySlopeToPlayer(m_Fire, m_FireVelocityY, m_FireOnGround, fireDx);

    std::vector<std::shared_ptr<Util::GameObject>> boxCollisionGroup = m_Stones;
    if (m_Gear) boxCollisionGroup.push_back(m_Gear);
    if (m_Gear2) boxCollisionGroup.push_back(m_Gear2);

    if (m_Box) {
        m_BoxOnGround = false;

        // 重力
        m_BoxVelocityY -= m_Gravity;
        m_Box->m_Transform.translation.y += m_BoxVelocityY;

        for (const auto &obj: boxCollisionGroup) {
            if (!obj) continue;

            if (IsColliding(m_Box, obj)) {
                float objTop = obj->m_Transform.translation.y + obj->GetScaledSize().y / 2.0f;
                float objBottom = obj->m_Transform.translation.y - obj->GetScaledSize().y / 2.0f;
                float boxHalfH = m_Box->GetScaledSize().y / 2.0f;

                if (m_BoxVelocityY <= 0.0f &&
                    m_Box->m_Transform.translation.y > obj->m_Transform.translation.y) {
                    // 落地
                    m_Box->m_Transform.translation.y = objTop + boxHalfH;
                    m_BoxVelocityY = 0.0f;
                    m_BoxOnGround = true;
                } else if (m_BoxVelocityY > 0.0f &&
                           m_Box->m_Transform.translation.y < obj->m_Transform.translation.y) {
                    // 頂頭
                    m_Box->m_Transform.translation.y = objBottom - boxHalfH;
                    m_BoxVelocityY = 0.0f;
                }
            }
        }
    }


    // 機關邏輯
    auto isStandingOnTop = [&](std::shared_ptr<Util::GameObject> character,
                               std::shared_ptr<Util::GameObject> platform) {
        if (!character || !platform) return false;

        float charHalfW = character->GetScaledSize().x / 2.0f;
        float charHalfH = character->GetScaledSize().y / 2.0f;
        float platHalfW = platform->GetScaledSize().x / 2.0f;
        float platHalfH = platform->GetScaledSize().y / 2.0f;

        float charLeft = character->m_Transform.translation.x - charHalfW;
        float charRight = character->m_Transform.translation.x + charHalfW;
        float charBottom = character->m_Transform.translation.y - charHalfH;

        float platLeft = platform->m_Transform.translation.x - platHalfW;
        float platRight = platform->m_Transform.translation.x + platHalfW;
        float platTop = platform->m_Transform.translation.y + platHalfH;

        bool overlapX = (charRight > platLeft) && (charLeft < platRight);
        bool onTop = std::abs(charBottom - platTop) < 5.0f;

        return overlapX && onTop;
    };

    bool isPressed =
        (m_Button && IsColliding(m_Ice, m_Button)) ||
        (m_Button && IsColliding(m_Fire, m_Button)) ||
        (m_Box && m_Button && IsColliding(m_Box, m_Button));

    bool iceOnGear = m_Gear && isStandingOnTop(m_Ice, m_Gear);
    bool fireOnGear = m_Gear && isStandingOnTop(m_Fire, m_Gear);

    glm::vec2 oldGearPos = m_Gear
        ? m_Gear->m_Transform.translation
        : glm::vec2(0.0f, 0.0f);

    if (m_Gear) {
        if (isPressed) {
            if (m_Button) m_Button->SetVisible(false);
            m_Gear->m_Transform.translation.y = m_GearOriginalPos.y + 50.0f;   // 往上移
        } else {
            if (m_Button) m_Button->SetVisible(true);
            m_Gear->m_Transform.translation.y = m_GearOriginalPos.y;            // 回原位
        }
    }

    glm::vec2 gearDelta = m_Gear
        ? (m_Gear->m_Transform.translation - oldGearPos)
        : glm::vec2(0.0f, 0.0f);

    if (iceOnGear) {
        m_Ice->m_Transform.translation += gearDelta;
    }
    if (fireOnGear) {
        m_Fire->m_Transform.translation += gearDelta;
    }

    //door
    bool iceAtDoor = IsColliding(m_Ice, m_IceDoor);
    bool fireAtDoor = IsColliding(m_Fire, m_FireDoor);


    //拉桿邏輯：判斷角色碰到拉桿的哪一側
    auto handleSwitch = [&](std::shared_ptr<Util::GameObject> character, float dx, bool isIce) {
        if (IsColliding(character, m_Switch)) {
            float charX = character->m_Transform.translation.x;
            float swX = m_Switch->m_Transform.translation.x;

            bool pushingRight = isIce ? keys[SDL_SCANCODE_D] : keys[SDL_SCANCODE_RIGHT];
            bool pushingLeft = isIce ? keys[SDL_SCANCODE_A] : keys[SDL_SCANCODE_LEFT];


            if (charX < swX && dx > 0 && pushingRight && m_IsSwitchOn) {
                m_IsSwitchOn = false;
                m_Switch->SetDrawable(std::make_shared<Util::Image>(PIC_PATH + "switch1_1.png"));
            }
            else if (charX > swX && dx < 0 && pushingLeft && !m_IsSwitchOn) {
                m_IsSwitchOn = true;
                m_Switch->SetDrawable(std::make_shared<Util::Image>(PIC_PATH + "switch1_2.png"));
            }
        }
    };

    // 呼叫時傳入 dx 與身份標記
    handleSwitch(m_Ice, iceDx, true);
    handleSwitch(m_Fire, fireDx, false);

    // 先判斷角色有沒有站在 Gear2 上
    bool iceOnGear2 = m_Gear2 && isStandingOnTop(m_Ice, m_Gear2);
    bool fireOnGear2 = m_Gear2 && isStandingOnTop(m_Fire, m_Gear2);



    // 記錄 Gear2 舊位置
    glm::vec2 oldGear2Pos = m_Gear2
        ? m_Gear2->m_Transform.translation
        : glm::vec2(0.0f, 0.0f);

    // 根據拉桿狀態移動 Gear2（上下）
    if (m_Gear2) {
        float targetY = m_IsSwitchOn
            ? (m_Gear2OriginalPos.y + 50.0f)
            : m_Gear2OriginalPos.y;

        float speed = 2.0f;

        if (m_Gear2->m_Transform.translation.y < targetY) {
            m_Gear2->m_Transform.translation.y += speed;
            if (m_Gear2->m_Transform.translation.y > targetY) {
                m_Gear2->m_Transform.translation.y = targetY;
            }
        }
        else if (m_Gear2->m_Transform.translation.y > targetY) {
            m_Gear2->m_Transform.translation.y -= speed;
            if (m_Gear2->m_Transform.translation.y < targetY) {
                m_Gear2->m_Transform.translation.y = targetY;
            }
        }
    }

    // 算這幀移動量
    glm::vec2 gear2Delta = m_Gear2
        ? (m_Gear2->m_Transform.translation - oldGear2Pos)
        : glm::vec2(0.0f, 0.0f);

    // 只有真的有移動才帶角色
    if (gear2Delta.x != 0.0f || gear2Delta.y != 0.0f) {
        if (iceOnGear2) {
            m_Ice->m_Transform.translation += gear2Delta;
            m_IceVelocityY = 0.0f;
            m_IceOnGround = true;
        }
        if (fireOnGear2) {
            m_Fire->m_Transform.translation += gear2Delta;
            m_FireVelocityY = 0.0f;
            m_FireOnGround = true;
        }
    }

    // 控制開關
    m_IceDoorOpening = iceAtDoor;
    m_FireDoorOpening = fireAtDoor;
    m_DoorAnimCounter++;

    if (m_DoorAnimCounter >= m_DoorAnimSpeed) {
        m_DoorAnimCounter = 0;

        // ===== 冰門 =====
        if (m_IceDoorOpening) {
            if (m_IceDoorFrameIndex < (int)m_IceDoorFrames.size() - 1) {
                m_IceDoorFrameIndex++;
            }
        } else {
            if (m_IceDoorFrameIndex > 0) {
                m_IceDoorFrameIndex--;
            }
        }

        m_IceDoor->SetDrawable(
            std::make_shared<Util::Image>(m_IceDoorFrames[m_IceDoorFrameIndex])
        );

        // ===== 火門 =====
        if (m_FireDoorOpening) {
            if (m_FireDoorFrameIndex < (int)m_FireDoorFrames.size() - 1) {
                m_FireDoorFrameIndex++;
            }
        } else {
            if (m_FireDoorFrameIndex > 0) {
                m_FireDoorFrameIndex--;
            }
        }

        m_FireDoor->SetDrawable(
            std::make_shared<Util::Image>(m_FireDoorFrames[m_FireDoorFrameIndex])
        );
    }

    // 5. 過關與死亡判定
    bool iceDoorOpen = (m_IceDoorFrameIndex == (int)m_IceDoorFrames.size() - 1);
    bool fireDoorOpen = (m_FireDoorFrameIndex == (int)m_FireDoorFrames.size() - 1);

    // 修正後的過關判定
    if (iceDoorOpen && fireDoorOpen && IsColliding(m_Ice, m_IceDoor) && IsColliding(m_Fire, m_FireDoor)) {
        LoadLevel(m_CurrentLevelNum + 1);
        return;
    }

    bool iceDead = false;
    bool fireDead = false;

    // 情況 A：Ice 的死亡判定
    if (IsColliding(m_Ice, m_Trap)) iceDead = true;
    if (IsColliding(m_Ice, m_IceTrap)) iceDead = true;

    // 情況 B：Fire 的死亡判定
    if (IsColliding(m_Fire, m_Trap)) fireDead = true;
    if (IsColliding(m_Fire, m_FireTrap)) fireDead = true;

    // 執行死亡效果
    if (iceDead || fireDead) {
        m_CurrentState = State::DEAD;
        m_DeadScreen->SetVisible(true);
    }
}