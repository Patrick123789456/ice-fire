#include "App.hpp"
#include "Util/Image.hpp"
#include "slope.hpp"

void App::LoadLevel(int level) {
    ClearLevel(); // 先清理舊的
    m_CurrentLevelNum = level;
    m_Score = 0;

    // 依照關卡編號設定地形
    if (level == 1 || level == 2) {
        // --- 1. 地板與障礙物 ---
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -640.0f + (i * stone->GetScaledSize().x), -200.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        for (int i = 0; i < 5; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -640.0f + (i * stone->GetScaledSize().x), -170.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        // --- 2. 角色位置重置 ---
        m_Ice->m_Transform.translation = { -500.0f, 0.0f };
        m_Fire->m_Transform.translation = { 500.0f, 0.0f };
        m_IceVelocityY = 0;
        m_FireVelocityY = 0;

        // --- 3. 門與機關 ---
        m_IceDoor->m_Transform.translation = { -300.0f, -141.0f };
        m_FireDoor->m_Transform.translation = { 300.0f, -141.0f };

        m_IceDoorFrameIndex = 0;
        m_FireDoorFrameIndex = 0;
        m_IceDoor->SetDrawable(std::make_shared<Util::Image>(m_IceDoorFrames[0]));
        m_FireDoor->SetDrawable(std::make_shared<Util::Image>(m_FireDoorFrames[0]));

        m_Trap->m_Transform.translation = { 0.0f, -171.0f };
        m_IceTrap->m_Transform.translation = { -350.0f, -171.0f };
        m_FireTrap->m_Transform.translation = { 350.0f, -171.0f };

        m_Button = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.5f);
        m_Button->m_Transform.translation = { 150.0f, -185.0f };
        m_Root->AddChild(m_Button);

        m_Gear = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear1.png"), -1.0f);
        m_Gear->m_Transform.translation = { 400.0f, -100.0f };
        m_GearOriginalPos = m_Gear->m_Transform.translation;
        m_Root->AddChild(m_Gear);

        if (!m_Box) {
            m_Box = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "box.png"), 0.1f);
            m_Root->AddChild(m_Box);
        }
        m_Box->m_Transform.translation = { -100.0f, -171.0f };
    }

    // --- 4. 共通物件 (拉桿、鑽石) ---
    m_Switch = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "switch1_1.png"), -1.5f);
    m_Switch->m_Transform.translation = { -200.0f, -185.0f };
    m_Root->AddChild(m_Switch);

    m_Gear2 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear2.png"), -1.0f);
    m_Gear2->m_Transform.translation = { -400.0f, -50.0f };
    m_Gear2OriginalPos = m_Gear2->m_Transform.translation;
    m_Root->AddChild(m_Gear2);

    //鑽石初始化
    InitDiamonds();

    // --- 5. 斜坡 ---
    AddSlope(PIC_PATH + "l_tri.png", {500.0f, -179.0f}, {1.2f, 1.2f}, {-14.0f, -14.0f}, {14.0f, 14.0f}, 0.2f, 0.2f, true);
    AddSlope(PIC_PATH + "r_tri.png", {100.0f, -179.0f}, {1.2f, 1.2f}, {-14.0f, 14.0f}, {14.0f, -14.0f}, 0.2f, 0.1f, true);

}

void App::ClearLevel() {
    // 清理石頭
    for (auto& stone : m_Stones) m_Root->RemoveChild(stone);
    m_Stones.clear();

    // 清理一般物件
    auto cleanup = [&](std::shared_ptr<Util::GameObject>& obj) {
        if (obj) { m_Root->RemoveChild(obj); obj = nullptr; }
    };
    cleanup(m_Button); cleanup(m_Gear); cleanup(m_Gear2);
    cleanup(m_Switch); cleanup(m_RedDiamond); cleanup(m_BlueDiamond);

    // 清理斜坡圖片
    for (auto& slope : m_Slopes) {
        if (slope.GetImage()) m_Root->RemoveChild(slope.GetImage());
    }
    m_Slopes.clear();
    m_IsSwitchOn = false;
}