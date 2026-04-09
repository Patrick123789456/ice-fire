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

void App::AddSlope(const std::string& imagePath,
                   const glm::vec2& imagePos,
                   const glm::vec2& imageScale,
                   const glm::vec2& localStart,
                   const glm::vec2& localEnd,
                   float slideSpeed,
                   float moveFactor,
                   bool isSolid,
                   float zIndex) {
    auto image = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(imagePath), zIndex
    );

    image->m_Transform.translation = imagePos;
    image->m_Transform.scale = imageScale;
    m_Root->AddChild(image);

    glm::vec2 start = imagePos + glm::vec2(localStart.x * imageScale.x,
                                       localStart.y * imageScale.y);

    glm::vec2 end = imagePos + glm::vec2(localEnd.x * imageScale.x,
                                         localEnd.y * imageScale.y);

    m_Slopes.emplace_back(image, start, end, slideSpeed, moveFactor, isSolid);
}

void App::ApplySlopeToPlayer(const std::shared_ptr<Util::GameObject>& player,
                             float& velocityY,
                             bool& onGround,
                             float& dx) {
    float currentX = player->m_Transform.translation.x;
    float currentY = player->m_Transform.translation.y;
    float footY = currentY - m_FootOffset;

    for (const auto& slope : m_Slopes) {
        if (!slope.IsSolid()) continue;

        float minX = std::min(slope.GetStartX(), slope.GetEndX());
        float maxX = std::max(slope.GetStartX(), slope.GetEndX());

        float currentSurfaceY = slope.GetSurfaceY(currentX);
        bool alreadyOnSlope =
            currentX >= minX && currentX <= maxX &&
            std::abs(footY - currentSurfaceY) <= m_SlopeTolerance;

        float moveDx = dx;
        if (std::abs(moveDx) >= 0.001f) {
            moveDx *= slope.GetMoveFactor();
        } else {
            moveDx = slope.IsLeftLowRightHigh()
                     ? -slope.GetSlideSpeed()
                     :  slope.GetSlideSpeed();
        }

        float nextX = currentX + moveDx;
        if (nextX < minX || nextX > maxX) {
            if (alreadyOnSlope) {
                player->m_Transform.translation.x = nextX;
                return;
            }
            continue;
        }

        float nextSurfaceY = slope.GetSurfaceY(nextX);
        float nextFootY = footY + velocityY;

        bool crossedSlope =
            footY >= nextSurfaceY &&   // 上一幀在坡上
            nextFootY <= nextSurfaceY; // 下一幀在坡下

        bool canSnapToSlope =
            nextFootY >= nextSurfaceY - m_SlopeTolerance &&
            nextFootY <= nextSurfaceY + m_SlopeSnapHeight;

        if (alreadyOnSlope || canSnapToSlope || crossedSlope) {
            onGround = true;
            velocityY = 0.0f;
            player->m_Transform.translation.x = nextX;
            player->m_Transform.translation.y = nextSurfaceY + m_FootOffset;
            return;
        }
    }

    player->m_Transform.translation.x += dx;
}

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
    m_IceTrap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap1.png"), -1.0f);
    m_FireTrap = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "trap2.png"), -1.0f);
    m_Root->AddChild(m_IceTrap);
    m_Root->AddChild(m_FireTrap);
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
    m_IcePosText->m_Transform.translation = { -560.0f, 340.0f };
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


    // 依照關卡編號設定地形 (目前第二關與第一關相同)
    if (level == 1 || level == 2) {
        for (int i = 0; i < 50; ++i) {
            auto stone = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
            stone->m_Transform.translation = { -640.0f + (i * stone->GetScaledSize().x), -200.0f };
            m_Stones.push_back(stone);
            m_Root->AddChild(stone);
        }
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
        //設定陷阱位置
        m_Trap->m_Transform.translation = { 0.0f, -171.0f };
        m_IceTrap->m_Transform.translation = { -350.0f, -171.0f };
        m_FireTrap->m_Transform.translation = { 350.0f, -171.0f };

        //設定機關位置
        m_Button = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.5f);
        m_Button->m_Transform.translation = { 150.0f, -185.0f }; // 放在地板上
        m_Root->AddChild(m_Button);

        m_Gear = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear1.png"), -1.0f);
        m_Gear->m_Transform.translation = { 400.0f, -100.0f }; // 機關位置
        m_GearOriginalPos = m_Gear->m_Transform.translation;
        m_Root->AddChild(m_Gear);

        if (!m_Box) {
            m_Box = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "box.png"), 0.1f); // 層級比角色高一點
            m_Root->AddChild(m_Box);
        }
        m_Box->m_Transform.translation = { -100.0f, -171.0f };
    }
    m_Switch = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "switch1_1.png"), -1.5f
    );
    m_Switch->m_Transform.translation = { -200.0f, -185.0f };
    m_Root->AddChild(m_Switch);

    m_Gear2 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "gear2.png"), -1.0f
    );
    m_Gear2->m_Transform.translation = { -400.0f, -50.0f };
    m_Gear2OriginalPos = m_Gear2->m_Transform.translation;
    m_Root->AddChild(m_Gear2);
    //diamond
    m_Score = 0;
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

    AddSlope(PIC_PATH + "l_tri.png",
         {500.0f, -179.0f},
         {1.2f, 1.2f},
         {-14.0f, -14.0f},   // localStart
         { 14.0f,14.0f},   // localEnd
         0.2f,              // slideSpeed
         0.2f,              // moveFactor
         true);             // isSolid

    AddSlope(PIC_PATH + "r_tri.png",
         {100.0f, -179.0f},
         {1.2f, 1.2f},
         {-14.0f,14.0f},   // localStart
         { 14.0f, -14.0f},   // localEnd
         0.2f,              // slideSpeed
         0.1f,              // moveFactor
         true);             // isSolid

    AddSlope(PIC_PATH + "r_tri.png",
         {75.0f, -152.0f},
         {1.2f, 1.2f},
         {-14.0f,14.0f},   // localStart
         { 14.0f, -14.0f},   // localEnd
         0.2f,              // slideSpeed
         0.1f,              // moveFactor
         true);




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
        collisionGroup.push_back(m_Box);
        collisionGroup.push_back(m_Gear);
        collisionGroup.push_back(m_Gear2);

        for (const auto& obj : collisionGroup) {
            if (!obj) continue;
            float objTop = obj->m_Transform.translation.y + (obj->GetScaledSize().y / 2.0f);
            float objBottom = obj->m_Transform.translation.y - (obj->GetScaledSize().y / 2.0f);

            if (IsColliding(m_Ice, obj)) {
                // 垂直判定：腳踏 (由上往下掉)
                if (m_IceVelocityY <= 0 && m_Ice->m_Transform.translation.y > obj->m_Transform.translation.y) {
                    m_IceVelocityY = 0;
                    m_Ice->m_Transform.translation.y = objTop + (m_Ice->GetScaledSize().y / 2.0f);
                    iG = true;
                }
                // 垂直判定：頂頭 (由下往上升) -> 這是新增的邏輯
                else if (m_IceVelocityY > 0 && m_Ice->m_Transform.translation.y < obj->m_Transform.translation.y) {
                    m_IceVelocityY = 0;
                    m_Ice->m_Transform.translation.y = objBottom - (m_Ice->GetScaledSize().y / 2.0f);
                }
            }

            if (IsColliding(m_Fire, obj)) {
                if (m_FireVelocityY <= 0 && m_Fire->m_Transform.translation.y > obj->m_Transform.translation.y) {
                    m_FireVelocityY = 0;
                    m_Fire->m_Transform.translation.y = objTop + (m_Fire->GetScaledSize().y / 2.0f);
                    fG = true;
                }
                else if (m_FireVelocityY > 0 && m_Fire->m_Transform.translation.y < obj->m_Transform.translation.y) {
                    m_FireVelocityY = 0;
                    m_Fire->m_Transform.translation.y = objBottom - (m_Fire->GetScaledSize().y / 2.0f);
                }
            }
        }
        m_IceOnGround = iG; m_FireOnGround = fG;
        ApplySlopeToPlayer(m_Ice, m_IceVelocityY, m_IceOnGround, iceDx);
        ApplySlopeToPlayer(m_Fire, m_FireVelocityY, m_FireOnGround, fireDx);

        // 機關邏輯
        bool isPressed = IsColliding(m_Ice, m_Button) || IsColliding(m_Fire, m_Button) || IsColliding(m_Box, m_Button);
        if (isPressed) {
            m_Button->SetVisible(false);
            m_Gear->m_Transform.translation.x = m_GearOriginalPos.x + 50.0f;
        } else {
            m_Button->SetVisible(true);
            m_Gear->m_Transform.translation.x = m_GearOriginalPos.x;
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


        // 根據拉桿狀態移動 Gear2
        if (m_IsSwitchOn) {
            m_Gear2->m_Transform.translation.x = m_Gear2OriginalPos.x - 50.0f;
        } else {
            m_Gear2->m_Transform.translation.x = m_Gear2OriginalPos.x;
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
        m_ScoreText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf",24,"Score: " + std::to_string(m_Score),Util::Color(255, 255, 0)));

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

    auto cleanup = [&](std::shared_ptr<Util::GameObject>& obj) {
        if (obj) {
            m_Root->RemoveChild(obj);
            obj = nullptr;
        }
    };

    cleanup(m_Button);
    cleanup(m_Gear);
    cleanup(m_Gear2);
    cleanup(m_Switch);
    cleanup(m_RedDiamond);
    cleanup(m_BlueDiamond);


    if (m_RedDiamond) {
        m_Root->RemoveChild(m_RedDiamond);
        m_RedDiamond = nullptr;
    }

    if (m_BlueDiamond) {
        m_Root->RemoveChild(m_BlueDiamond);
        m_BlueDiamond = nullptr;
    }

    for (auto& slope : m_Slopes) {
        if (slope.GetImage()) {
            m_Root->RemoveChild(slope.GetImage());
        }
    }
    m_Slopes.clear();
    

    m_IsSwitchOn = false;

}

void App::End() { LOG_TRACE("End"); }