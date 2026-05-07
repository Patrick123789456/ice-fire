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

    if (keys[SDL_SCANCODE_A]) iceDx -= m_MoveSpeed;
    if (keys[SDL_SCANCODE_D]) iceDx += m_MoveSpeed;
    if (Util::Input::IsKeyDown(Util::Keycode::W) && m_IceOnGround) { m_IceVelocityY = m_JumpForce; m_IceOnGround = false; }

    if (keys[SDL_SCANCODE_LEFT]) fireDx -= m_MoveSpeed;
    if (keys[SDL_SCANCODE_RIGHT]) fireDx += m_MoveSpeed;
    if (Util::Input::IsKeyDown(Util::Keycode::UP) && m_FireOnGround) { m_FireVelocityY = m_JumpForce; m_FireOnGround = false; }

    // 呼叫物理與機關 (實作在 App_Physics.cpp)
    HandleMechanics(iceDx, fireDx, keys);

    if (m_Fan) {
        m_Fan->ApplyWind(m_Ice, m_IceVelocityY, m_IceOnGround);
        m_Fan->ApplyWind(m_Fire, m_FireVelocityY, m_FireOnGround);
    }

    // 3. 動畫與門的邏輯
    UpdateAnimations();
    //確認寶石收集
    CheckDiamondCollection();

    // 4. 更新 UI
    m_IcePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, "Ice: (" + std::to_string((int)m_Ice->m_Transform.translation.x) + "," + std::to_string((int)m_Ice->m_Transform.translation.y) + ")", Util::Color(51,153,255)));
    m_FirePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, "Fire: (" + std::to_string((int)m_Fire->m_Transform.translation.x) + "," + std::to_string((int)m_Fire->m_Transform.translation.y) + ")", Util::Color(255,0,0)));
    m_ScoreText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 24, "Score: " + std::to_string(m_Score), Util::Color(255, 255, 0)));

    m_Root->Update();
}

void App::UpdateAnimations() {
    m_DoorAnimCounter++;
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