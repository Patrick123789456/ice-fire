#include "App.hpp"
#include "Util/Image.hpp"
#include "slope.hpp"

void App::LoadLevel(int level) {
    ClearLevel(); // 先清理舊的
    m_CurrentLevelNum = level;
    m_Score = 0;

    if (level == 1 || level == 2) {
        {//下
        for (int i = 0; i < 60; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + (i * stone->GetScaledSize().x), -349.0f }; // 移到最底部
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        //上
        for (int i = 0; i < 60; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + (i * stone->GetScaledSize().x), 349.0f }; // 視窗頂部
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        //左
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f, -349.0f + (i * stone->GetScaledSize().y) };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        //右
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { 629.0f, -349.0f + (i * stone->GetScaledSize().y) };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        for (int i = 0; i < 25; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + (i * stone->GetScaledSize().x), -326.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        for (int i = 0; i < 6; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + 30 * 23.0f + (i * stone->GetScaledSize().x), -326.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        for (int i = 0; i < 20; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + 41 * 23.0f + (i * stone->GetScaledSize().x), -326.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        for (int i = 0; i < 8; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + (i * stone->GetScaledSize().x), -257.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        //右下角突出
        {
        // 第一顆需要 auto 定義
        auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
        stone->m_Transform.translation = { 613.0f, -303.0f };
        m_Stones.push_back(stone);
        m_Root->AddChild(stone);

        // 後面幾顆直接指派新物件給同一個變數即可，不要再寫 auto
        stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
        stone->m_Transform.translation = { 613.0f, -280.0f };
        m_Stones.push_back(stone);
        m_Root->AddChild(stone);

        stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
        stone->m_Transform.translation = { 590.0f, -303.0f };
        m_Stones.push_back(stone);
        m_Root->AddChild(stone);

        stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
        stone->m_Transform.translation = { 590.0f, -280.0f };
        m_Stones.push_back(stone);
        m_Root->AddChild(stone);
        }

        //2F下
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + (i * stone->GetScaledSize().x), -165.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        //2F上
        for (int i = 0; i < 30; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f  + (i * stone->GetScaledSize().x), -142.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        for (int i = 0; i < 11; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + ((i+34) * stone->GetScaledSize().x), -142.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        //2.5F
        for (int i = 0; i < 56; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + ((i+4) * stone->GetScaledSize().x), -27.0f }; // 視窗頂部
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        //3F下
        for (int i = 0; i < 51; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -629.0f + (i * stone->GetScaledSize().x), 65.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        //3F中
        for (int i = 0; i < 20; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -54.0f + (i * stone->GetScaledSize().x), 88.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        //3F上
        for (int i = 0; i < 15; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -54.0f + (i * stone->GetScaledSize().x), 111.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }

        //4F
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -399.0f + (i * stone->GetScaledSize().x), 180.0f }; // 視窗頂部
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -399.0f + (i * stone->GetScaledSize().x), 203.0f }; // 視窗頂部
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        // 取得石磚尺寸 (假設寬度約 23.0f, 高度約 23.0f)
        auto tempStone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
        float sw = tempStone->GetScaledSize().x;
        float sh = tempStone->GetScaledSize().y;

        // --- 區域 1：X(-399~100), Y(134~157), 13x2 的方塊 ---
        for (int j = 1; j < 2; ++j) { // Y 軸 2 層
            for (int i = 0; i < 13; ++i) { // X 軸 13 顆
                auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
                stone->m_Transform.translation = { -399.0f + (i * sw), 134.0f + (j * sh) };
                m_Stones.push_back(stone);
                m_Root->AddChild(stone);
            }
        }

        // --- 區域 2：X(-606~-537), Y(88~157), 4x4 的方塊 ---
        for (int j = 0; j < 4; ++j) { // Y 軸 4 層
            for (int i = 0; i < 4; ++i) { // X 軸 4 顆
                auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
                stone->m_Transform.translation = { -606.0f + (i * sw), 88.0f + (j * sh) };
                m_Stones.push_back(stone);
                m_Root->AddChild(stone);
            }
        }

        // --- 區域 3：X(-399~-284), Y(180~249), 4x3 的方塊 ---
        for (int j = 0; j < 3; ++j) { // Y 軸 3 層
            for (int i = 0; i < 4; ++i) { // X 軸 4 顆
                auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
                stone->m_Transform.translation = { -399.0f + (i * sw), 180.0f + (j * sh) };
                m_Stones.push_back(stone);
                m_Root->AddChild(stone);
            }
        }

        // --- 填入陷阱 (IceTrap) ---
        for (int i = 0; i < 5; ++i) {
            auto trap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap1.png"), -1.0f);
            trap->m_Transform.translation = { -629.0f + (25 + i) * trap->GetScaledSize().x, -334.0f };
            m_Root->AddChild(trap);
            m_IceTraps.push_back(trap);
        }

        // --- 填入陷阱 (FireTrap) ---
        for (int i = 0; i < 5; ++i) {
            auto trap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap2.png"), -1.0f);
            trap->m_Transform.translation = { -629.0f + (36 + i) * trap->GetScaledSize().x, -334.0f };
            m_Root->AddChild(trap);
            m_FireTraps.push_back(trap);
        }

        // --- 填入陷阱 (Trap) ---
        for (int i = 0; i < 4; ++i) {
            auto trap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap0.png"), -1.0f);
            trap->m_Transform.translation = { -560.0f + (36 + i - 9) * trap->GetScaledSize().x, -150.0f };
            m_Root->AddChild(trap);
            m_Traps.push_back(trap); // 關鍵：存進容器
        }
    }

        // --- 2. 角色位置重置 ---
        m_Ice->m_Transform.translation = { -550.0f, -228.0f };
        m_Fire->m_Transform.translation = { -550.0f, -297.0f };
        m_IceVelocityY = 0;
        m_FireVelocityY = 0;

        // --- 3. 門與機關 ---
        m_IceDoor->m_Transform.translation = { -200.0f, 249.0f };
        m_FireDoor->m_Transform.translation = { -100.0f, 249.0f };

        m_IceDoorFrameIndex = 0;
        m_FireDoorFrameIndex = 0;
        m_IceDoor->SetDrawable(std::make_shared<Util::Image>(m_IceDoorFrames[0]));
        m_FireDoor->SetDrawable(std::make_shared<Util::Image>(m_FireDoorFrames[0]));


        m_Button = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.5f);
        m_Button->m_Transform.translation = { -400.0f, -10.0f };
        m_Root->AddChild(m_Button);

        m_Button2 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.0f);
        m_Button2->m_Transform.translation = { 150.0f, 130.0f}; // 換位置
        m_Root->AddChild(m_Button2);

        m_Gear = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear1.png"), -1.0f);
        m_Gear->m_Transform.translation = { 567.0f, 65.0f };
        m_GearOriginalPos = m_Gear->m_Transform.translation;
        m_Root->AddChild(m_Gear);

        if (!m_Box) {
            m_Box = std::make_shared<Util::GameObject>(
                std::make_shared<Util::Image>(PIC_PATH + "box.png"), 0.1f
            );
            m_Root->AddChild(m_Box);
        }
        m_BoxVelocityY = 0.0f;
        m_BoxOnGround = false;

        m_Box->m_Transform.translation = { -31.0f, 140.0f };

        // --- 4. 拉桿、鑽石 ---
        m_Switch = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "switch1_1.png"), -1.5f);
        m_Switch->m_Transform.translation = { -200.0f, -130.0f };
        m_Root->AddChild(m_Switch);

        m_Gear2 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear2.png"), -1.0f);
        m_Gear2->m_Transform.translation = { -580.0f, -27.0f };
        m_Gear2OriginalPos = m_Gear2->m_Transform.translation;
        m_Gear2->m_Transform.rotation = glm::radians(-90.0f);   // 轉 90 度
        m_Root->AddChild(m_Gear2);

        //鑽石初始化
        InitDiamonds();

        // --- 5. 斜坡 ---
        //AddSlope(PIC_PATH + "l_tri.png", {500.0f, -179.0f}, {1.2f, 1.2f}, {-14.0f, -14.0f}, {14.0f, 14.0f}, 0.2f, 0.2f, true);
        AddSlope(PIC_PATH + "r_tri.png", {409.0f, -143.0f}, {1.2f, 1.2f}, {-14.0f, 14.0f}, {14.0f, -14.0f}, 0.2f, 0.1f, true);

    }
    if (level == 1) {
        m_Fan->SetActive(true);

        m_Fan->SetPosition(
            glm::vec2(150.0f, -300.0f),
            glm::vec2(0.0f, 120.0f)
        );

        m_Fan->SetScale(
            glm::vec2(0.45f, 0.45f),
            glm::vec2(0.45f, 0.85f)
        );
    }
    else {
        m_Fan->SetActive(false);
    }
}

void App::ClearLevel() {
    // 清理石頭
    for (auto& stone : m_Stones) m_Root->RemoveChild(stone);
    m_Stones.clear();

    // 清理一般物件
    auto cleanup = [&](std::shared_ptr<Util::GameObject>& obj) {
        if (obj) { m_Root->RemoveChild(obj); obj = nullptr; }
    };
    cleanup(m_Button); cleanup(m_Button2); cleanup(m_Gear); cleanup(m_Gear2);
    cleanup(m_Switch); cleanup(m_RedDiamond); cleanup(m_BlueDiamond);

    // 清理斜坡圖片
    for (auto& slope : m_Slopes) {
        if (slope.GetImage()) m_Root->RemoveChild(slope.GetImage());
    }
    m_Slopes.clear();
    m_Traps.clear();
    m_IceTraps.clear();
    m_FireTraps.clear();
    m_IsSwitchOn = false;
}