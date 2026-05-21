#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Text.hpp"


static bool IsInWindArea(
    const std::shared_ptr<Util::GameObject>& player,
    const std::shared_ptr<Util::GameObject>& fan,
    float windWidth,
    float windHeight
) {
    if (!player || !fan) return false;

    glm::vec2 p = player->m_Transform.translation;
    glm::vec2 f = fan->m_Transform.translation;

    float left = f.x - windWidth / 2.0f;
    float right = f.x + windWidth / 2.0f;
    float bottom = f.y;
    float top = f.y + windHeight;

    return p.x >= left &&
           p.x <= right &&
           p.y >= bottom &&
           p.y <= top;
}

void App::Update() {
    if (Util::Input::IfExit()) { m_CurrentState = State::END; return; }

    // 1. 死亡與暫停處理
    if (m_CurrentState == State::DEAD) {
        if (Util::Input::IsKeyDown(Util::Keycode::R)) {
            m_DeadScreen->SetVisible(false);
            LoadLevel(m_CurrentLevelNum);
            m_GameTime = 0.0f; // 重新開始關卡時，計時重置
            m_CurrentState = State::UPDATE;
        }
        m_Root->Update(); return;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_CurrentState = (m_CurrentState == State::UPDATE) ? State::PAUSE : State::UPDATE;
        m_PauseScreen->SetVisible(m_CurrentState == State::PAUSE);
    }
    if (m_CurrentState == State::PAUSE) { m_Root->Update(); return; }

    // 2. 正常遊戲邏輯
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    float iceDx = 0.0f, fireDx = 0.0f;

    // 每一幀累加時間 (假設遊戲鎖定在 60 FPS，每幀約為 1/60 秒 = 0.0166f)
    // 如果你的框架有提供 DeltaTime，用 m_DeltaTime 會更精確
    m_GameTime += 0.0166f; 

    // 走路按鍵判定與狀態標記
    m_IceIsWalking = false;
    if (keys[SDL_SCANCODE_A]) { iceDx -= m_MoveSpeed; m_IceFacingRight = false; m_IceIsWalking = true; }
    if (keys[SDL_SCANCODE_D]) { iceDx += m_MoveSpeed; m_IceFacingRight = true; m_IceIsWalking = true; }
    if (Util::Input::IsKeyDown(Util::Keycode::W) && m_IceOnGround) { m_IceVelocityY = m_JumpForce; m_IceOnGround = false; }

    m_FireIsWalking = false;
    if (keys[SDL_SCANCODE_LEFT]) { fireDx -= m_MoveSpeed; m_FireFacingRight = false; m_FireIsWalking = true; }
    if (keys[SDL_SCANCODE_RIGHT]) { fireDx += m_MoveSpeed; m_FireFacingRight = true; m_FireIsWalking = true; }
    if (Util::Input::IsKeyDown(Util::Keycode::UP) && m_FireOnGround) { m_FireVelocityY = m_JumpForce; m_FireOnGround = false; }

    // 呼叫物理與機關
    HandleMechanics(iceDx, fireDx, keys);

    if (m_Fan) {
        m_Fan->ApplyWind(m_Ice, m_IceVelocityY, m_IceOnGround);
        m_Fan->ApplyWind(m_Fire, m_FireVelocityY, m_FireOnGround);
    }

    // 3. 動畫與門的邏輯
    UpdateAnimations();
    CheckDiamondCollection();

    // 4. 更新 UI
    m_IcePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, "Ice: (" + std::to_string((int)m_Ice->m_Transform.translation.x) + "," + std::to_string((int)m_Ice->m_Transform.translation.y) + ")", Util::Color(51,153,255)));
    m_FirePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, "Fire: (" + std::to_string((int)m_Fire->m_Transform.translation.x) + "," + std::to_string((int)m_Fire->m_Transform.translation.y) + ")", Util::Color(255,0,0)));

    int minutes = (int)m_GameTime / 60;
    int seconds = (int)m_GameTime % 60;

    char timeBuffer[16];
    snprintf(timeBuffer, sizeof(timeBuffer), "Time: %02d:%02d", minutes, seconds);

    m_ScoreText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 24, timeBuffer, Util::Color(255, 255, 0)));

    m_Root->Update();
}

void App::UpdateAnimations() {
    m_DoorAnimCounter++;
    // 門動畫
    if (m_DoorAnimCounter >= m_DoorAnimSpeed) {
        m_DoorAnimCounter = 0;
        auto updateDoor = [&](bool opening, int& frame, std::vector<std::string>& frames, std::shared_ptr<Util::GameObject>& door) {
            if (opening && frame < (int)frames.size() - 1) frame++;
            else if (!opening && frame > 0) frame--;
            door->SetDrawable(std::make_shared<Util::Image>(frames[frame]));
        };
        updateDoor(IsColliding(m_Ice, m_IceDoor), m_IceDoorFrameIndex, m_IceDoorFrames, m_IceDoor);
        updateDoor(IsColliding(m_Fire, m_FireDoor), m_FireDoorFrameIndex, m_FireDoorFrames, m_FireDoor);
    }


    if (m_PlayerAnimCounter >= m_PlayerAnimSpeed) {

        m_PlayerAnimCounter++;
        m_PlayerAnimCounter = 0;

        // --- Ice 動畫邏輯 ---
        if (m_IceIsWalking) {
            // 有在走路，依照面向播放對應的走路動畫
            if (m_IceFacingRight) {
                m_IceWalkFrameIndex = (m_IceWalkFrameIndex + 1) % m_IceWalkFrames.size();
                m_Ice->SetDrawable(std::make_shared<Util::Image>(m_IceWalkFrames[m_IceWalkFrameIndex]));
            } else {
                m_IceWalkFrameIndex = (m_IceWalkFrameIndex + 1) % m_IceWalkFrames_left.size();
                m_Ice->SetDrawable(std::make_shared<Util::Image>(m_IceWalkFrames_left[m_IceWalkFrameIndex]));
            }
        } else {
            // 沒在走路，給予站立不動的靜態圖 (區分左右)
            m_Ice->SetDrawable(std::make_shared<Util::Image>(m_IceFacingRight ? PIC_PATH + "ice.png" : PIC_PATH + "ice_left.png"));
        }

        // --- Fire 動畫邏輯 ---
        if (m_FireIsWalking) {
            // 有在走路，依照面向播放對應的走路動畫
            if (m_FireFacingRight) {
                m_FireWalkFrameIndex = (m_FireWalkFrameIndex + 1) % m_FireWalkFrames.size();
                m_Fire->SetDrawable(std::make_shared<Util::Image>(m_FireWalkFrames[m_FireWalkFrameIndex]));
            } else {
                m_FireWalkFrameIndex = (m_FireWalkFrameIndex + 1) % m_FireWalkFrames_left.size();
                m_Fire->SetDrawable(std::make_shared<Util::Image>(m_FireWalkFrames_left[m_FireWalkFrameIndex]));
            }
        } else {
            // 沒在走路，給予站立不動的靜態圖 (區分左右)
            m_Fire->SetDrawable(std::make_shared<Util::Image>(m_FireFacingRight ? PIC_PATH + "fire.png" : PIC_PATH + "fire_left.png"));
        }
    }

    //陷阱動畫
    m_TrapAnimCounter++;
    if (m_TrapAnimCounter >= m_TrapAnimSpeed) {
        m_TrapAnimCounter = 0;

        // 更新當前動畫幀索引 (0~9 循環)
        m_TrapFrameIndex = (m_TrapFrameIndex + 1) % 10;

        // 更新所有冰陷阱的圖片
        for (auto& trap : m_IceTraps) {
            trap->SetDrawable(std::make_shared<Util::Image>(m_IceTrapFrames[m_TrapFrameIndex]));
        }

        // 更新所有火陷阱的圖片
        for (auto& trap : m_FireTraps) {
            trap->SetDrawable(std::make_shared<Util::Image>(m_FireTrapFrames[m_TrapFrameIndex]));
        }

        // 更新所有普通陷阱的圖片
        for (auto& trap : m_Traps) {
            trap->SetDrawable(std::make_shared<Util::Image>(m_TrapFrames[m_TrapFrameIndex]));
        }
    }


    // 電風扇動畫
    if (m_Fan) {
        m_Fan->UpdateAnimation();
    }


    // 鑽石浮動
    m_DiamondFloatTime += m_DiamondFloatSpeed;
    if (m_RedDiamond && !m_RedDiamondCollected)
        m_RedDiamond->m_Transform.translation.y = m_RedDiamondBasePos.y + std::sin(m_DiamondFloatTime) * m_DiamondFloatRange;
    if (m_BlueDiamond && !m_BlueDiamondCollected)
        m_BlueDiamond->m_Transform.translation.y = m_BlueDiamondBasePos.y + std::sin(m_DiamondFloatTime) * m_DiamondFloatRange;
}