#include "App.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"

void App::Start() {
    m_Root = std::make_shared<Util::Renderer>();

    // 1. 系統畫面 (永遠存在的 UI)
    m_PauseScreen = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stop.png"), 20.0f);
    m_PauseScreen->SetVisible(false);
    m_Root->AddChild(m_PauseScreen);

    m_DeadScreen = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "dead.png"), 25.0f);
    m_DeadScreen->SetVisible(false);
    m_Root->AddChild(m_DeadScreen);

    m_Background = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "background.png"), -10.0f);
    m_Root->AddChild(m_Background);

    // 2. 角色 (在 LoadLevel 中只會改座標，不會刪除)
    m_Ice = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "ice.png"), 0.0f);
    m_Fire = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "fire.png"), 0.0f);
    m_Root->AddChild(m_Ice);
    m_Root->AddChild(m_Fire);

    // 3. 陷阱與門 (預先載入，LoadLevel 再改位置)
    m_Trap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap0.png"), -1.0f);
    m_IceTrap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap1.png"), -1.0f);
    m_FireTrap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap2.png"), -1.0f);
    m_Root->AddChild(m_IceTrap);
    m_Root->AddChild(m_FireTrap);
    m_Root->AddChild(m_Trap);

    m_IceDoor = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "ice_door.png"), -2.0f);
    m_FireDoor = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "fire_door.png"), -2.0f);
    m_Root->AddChild(m_IceDoor);
    m_Root->AddChild(m_FireDoor);

    for (int i = 1; i <= 22; i++) {
        m_IceDoorFrames.push_back(PIC_PATH + "ice_door/ice_door" + std::to_string(i) + ".png");
        m_FireDoorFrames.push_back(PIC_PATH + "fire_door/fire_door" + std::to_string(i) + ".png");
    }

    // 4. 初始化 UI 文字
    std::string font = FONT_PATH + "arial.ttf";
    m_IcePosText = std::make_shared<Util::GameObject>(std::make_shared<Util::Text>(font, 20, "Ice: (0, 0)", Util::Color(51,153,255)), 10.0f);
    m_IcePosText->m_Transform.translation = { -560.0f, 340.0f };
    m_Root->AddChild(m_IcePosText);

    m_FirePosText = std::make_shared<Util::GameObject>(std::make_shared<Util::Text>(font, 20, "Fire: (0, 0)", Util::Color(255, 0, 0)), 10.0f);
    m_FirePosText->m_Transform.translation = { 570.0f, 340.0f };
    m_Root->AddChild(m_FirePosText);

    m_ScoreText = std::make_shared<Util::GameObject>(std::make_shared<Util::Text>(font, 24, "Score: 0", Util::Color(255, 255, 0)), 10.0f);
    m_ScoreText->m_Transform.translation = { 0.0f, 340.0f };
    m_Root->AddChild(m_ScoreText);

    // 5. 進入第一關
    LoadLevel(1);
    m_CurrentState = State::UPDATE;
}