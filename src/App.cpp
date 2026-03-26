#include "App.hpp"
#include "config.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Text.hpp"
#include "cmath"

const std::string PIC_PATH = "../Resources/picture/";
const std::string FONT_PATH = "../Resources/font/";

void App::Start() {
    m_Root = std::make_shared<Util::Renderer>();

    // 1. 暫停與死亡畫面 (層級最高)
    m_PauseScreen = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stop.png"), 20.0f);
    m_PauseScreen->SetVisible(false);
    m_Root->AddChild(m_PauseScreen);

    m_DeadScreen = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "dead.png"), 25.0f);
    m_DeadScreen->SetVisible(false);
    m_Root->AddChild(m_DeadScreen);

    // 2. 背景與角色
    m_Background = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "background.png"), -10.0f);
    m_Root->AddChild(m_Background);

    m_Ice = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "ice.png"), 0.0f);
    m_Fire = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "fire.png"), 0.0f);
    m_Root->AddChild(m_Ice);
    m_Root->AddChild(m_Fire);

    // 3. 地板與陷阱
    m_Trap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap0.png"), -1.0f);
    m_Root->AddChild(m_Trap);

    // 4. 門 (暫時放在這裡，之後會移到 LoadLevel)
    if (!m_IceDoor) {
        m_IceDoor = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "ice_door.png"), -2.0f);
        m_FireDoor = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "fire_door.png"), -2.0f);
        m_Root->AddChild(m_IceDoor);
        m_Root->AddChild(m_FireDoor);
    }

    // 4. UI 文字
    std::string font = FONT_PATH + "arial.ttf";
    m_IcePosText = std::make_shared<Util::GameObject>(std::make_shared<Util::Text>(font, 20, "Ice: (0, 0)", Util::Color(51,153,255)), 10.0f);
    m_IcePosText->SetPivot({-1.0f, 1.0f});
    m_IcePosText->m_Transform.translation = { -570.0f, 340.0f };
    m_Root->AddChild(m_IcePosText);

    m_FirePosText = std::make_shared<Util::GameObject>(std::make_shared<Util::Text>(font, 20, "Fire: (0, 0)", Util::Color(255, 0, 0)), 10.0f);
    m_FirePosText->SetPivot({1.0f, 1.0f});
    m_FirePosText->m_Transform.translation = { 570.0f, 340.0f };
    m_Root->AddChild(m_FirePosText);

    //diamond
    m_ScoreText = std::make_shared<Util::GameObject>(
    std::make_shared<Util::Text>(font, 24, "Score: 0", Util::Color(255, 255, 0)),
    10.0f
);
    m_ScoreText->SetPivot({0.0f, 1.0f});
    m_ScoreText->m_Transform.translation = { 0.0f, 340.0f };
    m_Root->AddChild(m_ScoreText);

    // ===== 門動畫圖片自動載入 =====
    for (int i = 1; i <= 22; i++) {
        m_IceDoorFrames.push_back(PIC_PATH + "ice_door/ice_door" + std::to_string(i) + ".png");
        m_FireDoorFrames.push_back(PIC_PATH + "fire_door/fire_door" + std::to_string(i) + ".png");
    }

    // 初始化第一關
    LoadLevel(1);
    m_CurrentState = State::UPDATE;
}

void App::LoadLevel(int level) {
    ClearLevel(); // 清理舊有的物件

    // 重置角色位置
    m_Ice->m_Transform.translation = { -500.0f, 0.0f };
    m_Fire->m_Transform.translation = { 500.0f, 0.0f };
    m_IceVelocityY = 0;
    m_FireVelocityY = 0;

    // 設定門的位置
    m_IceDoor->m_Transform.translation = { -300.0f, -141.0f };
    m_FireDoor->m_Transform.translation = { 300.0f, -141.0f };

    m_IceDoorFrameIndex = 0;
    m_FireDoorFrameIndex = 0;

    m_IceDoorOpening = false;
    m_FireDoorOpening = false;

    m_DoorAnimCounter = 0;

    // 設回關門圖片
    m_IceDoor->SetDrawable(std::make_shared<Util::Image>(m_IceDoorFrames[0]));
    m_FireDoor->SetDrawable(std::make_shared<Util::Image>(m_FireDoorFrames[0]));


    //設定機關位置
    m_Button = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.5f);
    m_Button->m_Transform.translation = { 150.0f, -185.0f }; // 放在地板上
    m_Root->AddChild(m_Button);

    m_Gear = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear1.png"), -1.0f);
    m_Gear->m_Transform.translation = { 400.0f, -100.0f }; // 機關位置
    m_GearOriginalPos = m_Gear->m_Transform.translation;
    m_Root->AddChild(m_Gear);

    // 依照關卡編號設定地形 (目前第二關與第一關相同)
    if (level == 1 || level == 2) {
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -640.0f + (i * stone->GetScaledSize().x), -200.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
        m_Trap->m_Transform.translation = { 0.0f, -171.0f };

        if (!m_Box) {
            m_Box = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "box.png"), 0.1f); // 層級比角色高一點
            m_Root->AddChild(m_Box);
        }
        m_Box->m_Transform.translation = { -100.0f, -171.0f };
    }

    //diamond
    m_RedDiamondCollected = false;
    m_BlueDiamondCollected = false;
    m_DiamondFloatTime = 0.0f;

    m_RedDiamond = std::make_shared<Util::GameObject>(
    std::make_shared<Util::Image>(PIC_PATH + "r_diamond.png"), 0.2f
);
    m_RedDiamond->m_Transform.translation = { 100.0f, -120.0f };
    m_RedDiamond->m_Transform.scale = { 0.6f, 0.6f };
    m_RedDiamond->SetVisible(true);
    m_Root->AddChild(m_RedDiamond);
    m_RedDiamondBasePos = m_RedDiamond->m_Transform.translation;

    m_BlueDiamond = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "b_diamond.png"), 0.2f
    );
    m_BlueDiamond->m_Transform.translation = { -100.0f, -120.0f };
    m_BlueDiamond->m_Transform.scale = { 0.6f, 0.6f };
    m_BlueDiamond->SetVisible(true);
    m_Root->AddChild(m_BlueDiamond);
    m_BlueDiamondBasePos = m_BlueDiamond->m_Transform.translation;

    m_CurrentLevelNum = level;
    LOG_INFO("Loading Level " + std::to_string(level));
}

void App::Update() {
    if (Util::Input::IfExit()) { m_CurrentState = State::END; return; }

    // --- 死亡狀態 ---
    if (m_CurrentState == State::DEAD) {
        if (Util::Input::IsKeyDown(Util::Keycode::R)) {
            m_DeadScreen->SetVisible(false);
            LoadLevel(m_CurrentLevelNum); 
            m_CurrentState = State::UPDATE;
        }
        m_Root->Update();
        return;
    }

    // --- 暫停邏輯 ---
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        if (m_CurrentState == State::UPDATE) {
            m_CurrentState = State::PAUSE;
            m_PauseScreen->SetVisible(true);
        } else if (m_CurrentState == State::PAUSE) {
            m_CurrentState = State::UPDATE;
            m_PauseScreen->SetVisible(false);
        }
    }
    if (m_CurrentState == State::PAUSE) { m_Root->Update(); return; }

    // --- 遊戲更新 ---
    if (m_CurrentState == State::UPDATE) {
        if (Util::Input::IsKeyDown(Util::Keycode::N)) {
            LoadLevel(m_CurrentLevelNum + 1);
            return;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float iceDx = 0.0f, fireDx = 0.0f;

        // 1. 取得輸入
        if (keys[SDL_SCANCODE_A]) iceDx -= m_MoveSpeed;
        if (keys[SDL_SCANCODE_D]) iceDx += m_MoveSpeed;
        if (Util::Input::IsKeyDown(Util::Keycode::W) && m_IceOnGround) { m_IceVelocityY = m_JumpForce; m_IceOnGround = false; }

        if (keys[SDL_SCANCODE_LEFT]) fireDx -= m_MoveSpeed;
        if (keys[SDL_SCANCODE_RIGHT]) fireDx += m_MoveSpeed;
        if (Util::Input::IsKeyDown(Util::Keycode::UP) && m_FireOnGround) { m_FireVelocityY = m_JumpForce; m_FireOnGround = false; }

        // 2. 處理水平推箱子 (在移動角色座標前先處理)
        auto handleAdvancedPush = [&]() {
            const float PUSH_SPEED = 3.5f;
            const float SANDWICH_SPEED = 2.0f;

            // --- 情況 A：Ice 推 Box ---
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
                    bool hitStone = false;
                    for (auto& s : m_Stones) { if (IsColliding(m_Box, s)) { hitStone = true; break; } }

                    if (hitStone) {
                        m_Box->m_Transform.translation.x -= finalDx;
                        if (isSandwich) m_Fire->m_Transform.translation.x -= finalDx;
                        iceDx = 0;
                    } else {
                        iceDx = finalDx;
                        if (isSandwich) fireDx = finalDx;
                    }
                }
            }

            // --- 情況 B：Fire 推 Box (同樣加入方向檢查) ---
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
                    bool hitStone = false;
                    for (auto& s : m_Stones) { if (IsColliding(m_Box, s)) { hitStone = true; break; } }

                    if (hitStone) {
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

        // 執行最後位移
        m_IceVelocityY -= m_Gravity;
        m_FireVelocityY -= m_Gravity;
        m_Ice->m_Transform.translation += glm::vec2(iceDx, m_IceVelocityY);
        m_Fire->m_Transform.translation += glm::vec2(fireDx, m_FireVelocityY);

        // 4. 地板與踩箱子碰撞
        bool iG = false, fG = false;

        // 建立一個臨時列表包含石頭和箱子，統一處理垂直碰撞
        for (const auto& stone : m_Stones) {
            float sTop = stone->m_Transform.translation.y + (stone->GetScaledSize().y / 2.0f);
            if (m_IceVelocityY <= 0 && IsColliding(m_Ice, stone)) {
                m_IceVelocityY = 0;
                m_Ice->m_Transform.translation.y = sTop + (m_Ice->GetScaledSize().y / 2.0f);
                iG = true;
            }
            if (m_FireVelocityY <= 0 && IsColliding(m_Fire, stone)) {
                m_FireVelocityY = 0;
                m_Fire->m_Transform.translation.y = sTop + (m_Fire->GetScaledSize().y / 2.0f);
                fG = true;
            }
        }

        // 處理踩在箱子上面 (只有 Y 軸速度向下且位置在箱子上方才判定)
        float bTop = m_Box->m_Transform.translation.y + (m_Box->GetScaledSize().y / 2.0f);
        if (m_IceVelocityY <= 0 && IsColliding(m_Ice, m_Box) && m_Ice->m_Transform.translation.y > m_Box->m_Transform.translation.y) {
            m_IceVelocityY = 0;
            m_Ice->m_Transform.translation.y = bTop + (m_Ice->GetScaledSize().y / 2.0f);
            iG = true;
        }
        if (m_FireVelocityY <= 0 && IsColliding(m_Fire, m_Box) && m_Fire->m_Transform.translation.y > m_Box->m_Transform.translation.y) {
            m_FireVelocityY = 0;
            m_Fire->m_Transform.translation.y = bTop + (m_Fire->GetScaledSize().y / 2.0f);
            fG = true;
        }

        m_IceOnGround = iG; m_FireOnGround = fG;

        //door
        bool iceAtDoor = IsColliding(m_Ice, m_IceDoor);
        bool fireAtDoor = IsColliding(m_Fire, m_FireDoor);

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



        //機關邏輯
        bool isPressed = IsColliding(m_Ice, m_Button) || 
                 IsColliding(m_Fire, m_Button) || 
                 IsColliding(m_Box, m_Button);

        if (isPressed) {
            // 按鈕被壓住：按鈕消失（或變色），機關往右移 50px
            m_Button->SetVisible(false);
            m_Gear->m_Transform.translation.x = m_GearOriginalPos.x + 50.0f;
        } else {
            // 沒有人壓住：按鈕出現，機關回到原位
            m_Button->SetVisible(true);
            m_Gear->m_Transform.translation.x = m_GearOriginalPos.x;
        }

        // 5. 過關與死亡判定
        bool iceDoorOpen = (m_IceDoorFrameIndex == (int)m_IceDoorFrames.size() - 1);
        bool fireDoorOpen = (m_FireDoorFrameIndex == (int)m_FireDoorFrames.size() - 1);

        if (iceAtDoor && fireAtDoor && iceDoorOpen && fireDoorOpen) {
            LoadLevel(m_CurrentLevelNum + 1);
            return;
        }

        if (IsColliding(m_Ice, m_Trap) || IsColliding(m_Fire, m_Trap)) {
            m_CurrentState = State::DEAD;
            m_DeadScreen->SetVisible(true);
        }

        //diamond
        if (!m_RedDiamondCollected && m_RedDiamond) {
            if (IsColliding(m_Fire, m_RedDiamond)) {
                m_RedDiamondCollected = true;
                m_RedDiamond->SetVisible(false);
                m_Score += 1;
            }
        }

        if (!m_BlueDiamondCollected && m_BlueDiamond) {
            if (IsColliding(m_Ice, m_BlueDiamond)) {
                m_BlueDiamondCollected = true;
                m_BlueDiamond->SetVisible(false);
                m_Score += 1;
            }
        }
        m_DiamondFloatTime += m_DiamondFloatSpeed;

        if (!m_RedDiamondCollected && m_RedDiamond) {
            m_RedDiamond->m_Transform.translation.y =
                m_RedDiamondBasePos.y + std::sin(m_DiamondFloatTime) * m_DiamondFloatRange;
        }

        if (!m_BlueDiamondCollected && m_BlueDiamond) {
            m_BlueDiamond->m_Transform.translation.y =
                m_BlueDiamondBasePos.y + std::sin(m_DiamondFloatTime) * m_DiamondFloatRange;
        }


        // 更新 UI
        m_IcePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, "Ice: (" + std::to_string((int)m_Ice->m_Transform.translation.x) + "," + std::to_string((int)m_Ice->m_Transform.translation.y) + ")", Util::Color(51,153,255)));
        m_FirePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, "Fire: (" + std::to_string((int)m_Fire->m_Transform.translation.x) + "," + std::to_string((int)m_Fire->m_Transform.translation.y) + ")", Util::Color(255,0,0)));
        m_ScoreText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf",24,"Score: " + std::to_string(m_Score),Util::Color(255, 255, 0)
    )
);
        m_Root->Update();
    }
}

bool App::IsColliding(const std::shared_ptr<Util::GameObject>& p, const std::shared_ptr<Util::GameObject>& t) {
    glm::vec2 pP = p->m_Transform.translation, pS = p->GetScaledSize();
    glm::vec2 tP = t->m_Transform.translation, tS = t->GetScaledSize();
    return (pP.x - pS.x / 2.1f < tP.x + tS.x / 2.0f && pP.x + pS.x / 2.1f > tP.x - tS.x / 2.0f &&
            pP.y - pS.y / 2.0f < tP.y + tS.y / 2.0f && pP.y + pS.y / 2.0f > tP.y - tS.y / 2.0f);
}

void App::ClearLevel() {
    for (auto& stone : m_Stones) {
        m_Root->RemoveChild(stone);
    }
    m_Stones.clear();

    if (m_Button) {
        m_Root->RemoveChild(m_Button);
        m_Button = nullptr;
    }

    if (m_Gear) {
        m_Root->RemoveChild(m_Gear);
        m_Gear = nullptr;
    }

    if (m_RedDiamond) {
        m_Root->RemoveChild(m_RedDiamond);
        m_RedDiamond = nullptr;
    }

    if (m_BlueDiamond) {
        m_Root->RemoveChild(m_BlueDiamond);
        m_BlueDiamond = nullptr;
    }
}

void App::End() { LOG_TRACE("End"); }