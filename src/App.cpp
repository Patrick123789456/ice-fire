#include "App.hpp"
#include "config.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Text.hpp"
#include <string>


void App::Start() {
    LOG_TRACE("Start");
    m_Root = std::make_shared<Util::Renderer>();

    // 1. 背景初始化
    m_Background = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "background.png"), -10.0f);
    m_Root->AddChild(m_Background);

    // 2. 角色初始化
    m_Ice = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "ice.png"), 0.0f);
    m_Root->AddChild(m_Ice);

    m_Fire = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "fire.png"), 0.0f);
    m_Root->AddChild(m_Fire);

    m_Ice->m_Transform.translation = { -500.0f, 0.0f }; // 初始位置
    m_Fire->m_Transform.translation = { 500.0f, 0.0f };  
    // 3. 石頭 (地面) 初始化
    // 在 App::Start() 中修改
for (int i = 0; i < 50; ++i) { // 稍微增加數量確保填滿 1280 寬度
    auto stone = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "stone1.png"), -1.0f);
    
    // 關鍵修正：i * 70.0f 可能太大了，請根據你 stone1.png 的實際寬度調整
    // 如果圖片寬度是 64，這裡就用 64.0f。 
    // 我們可以利用 GetScaledSize().x 來自動對齊：
    float stoneW = stone->GetScaledSize().x; 
    stone->m_Transform.translation = { -640.0f + (i * stoneW), -200.0f };
    
    m_Stones.push_back(stone);
    m_Root->AddChild(stone);
}

    // 4. 座標文字初始化 (請確保 Resources/font/ 下有 arial.ttf)
    std::string font = FONT_PATH + "arial.ttf";
    
    m_IcePosText = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Text>(font, 20, "Ice: (0, 0)", Util::Color(0, 255, 0)), 10.0f);
    m_IcePosText->SetPivot({-1.0f, 1.0f});
    m_IcePosText->m_Transform.translation = { -450.0f, 330.0f };
    m_Root->AddChild(m_IcePosText);

    m_FirePosText = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Text>(font, 20, "Fire: (0, 0)", Util::Color(255, 0, 0)), 10.0f);
    m_FirePosText->SetPivot({1.0f, 1.0f});
    m_FirePosText->m_Transform.translation = { 450.0f, 330.0f };
    m_Root->AddChild(m_FirePosText);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    // --- A. 獲取鍵盤快照 (解決按住不放的移動問題) ---
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    float iceDx = 0.0f, fireDx = 0.0f;
    
    // Ice 控制 (A, D, W)
    if (currentKeyStates[SDL_SCANCODE_A]) iceDx -= m_MoveSpeed;
    if (currentKeyStates[SDL_SCANCODE_D]) iceDx += m_MoveSpeed;
    if (Util::Input::IsKeyDown(Util::Keycode::W) && m_IceOnGround) {
        m_IceVelocityY = m_JumpForce;
        m_IceOnGround = false;
    }

    // Fire 控制 (Left, Right, Up)
    if (currentKeyStates[SDL_SCANCODE_LEFT]) fireDx -= m_MoveSpeed;
    if (currentKeyStates[SDL_SCANCODE_RIGHT]) fireDx += m_MoveSpeed;
    if (Util::Input::IsKeyDown(Util::Keycode::UP) && m_FireOnGround) {
        m_FireVelocityY = m_JumpForce;
        m_FireOnGround = false;
    }

    // --- B. 套用物理與位移 ---
    m_IceVelocityY -= m_Gravity;
    m_FireVelocityY -= m_Gravity;

    m_Ice->m_Transform.translation.x += iceDx;
    m_Fire->m_Transform.translation.x += fireDx;
    m_Ice->m_Transform.translation.y += m_IceVelocityY;
    m_Fire->m_Transform.translation.y += m_FireVelocityY;

    // --- C. 碰撞檢查與修正 ---
    bool iceTouching = false;
    bool fireTouching = false;

    for (const auto& stone : m_Stones) {
        float stoneTop = stone->m_Transform.translation.y + (stone->GetScaledSize().y / 2.0f);
        
        if (m_IceVelocityY <= 0 && IsColliding(m_Ice, stone)) {
            m_IceVelocityY = 0;
            m_Ice->m_Transform.translation.y = stoneTop + (m_Ice->GetScaledSize().y / 2.0f) + 0.1f;
            iceTouching = true;
        }
        if (m_FireVelocityY <= 0 && IsColliding(m_Fire, stone)) {
            m_FireVelocityY = 0;
            m_Fire->m_Transform.translation.y = stoneTop + (m_Fire->GetScaledSize().y / 2.0f) + 0.1f;
            fireTouching = true;
        }
    }
    m_IceOnGround = iceTouching;
    m_FireOnGround = fireTouching;

    // --- D. 更新座標文字內容 ---
    std::string iPos = "Ice: (" + std::to_string((int)m_Ice->m_Transform.translation.x) + 
                       ", " + std::to_string((int)m_Ice->m_Transform.translation.y) + ")";
    m_IcePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, iPos, Util::Color(51,153,255)));

    std::string fPos = "Fire: (" + std::to_string((int)m_Fire->m_Transform.translation.x) + 
                       ", " + std::to_string((int)m_Fire->m_Transform.translation.y) + ")";
    m_FirePosText->SetDrawable(std::make_shared<Util::Text>(FONT_PATH + "arial.ttf", 20, fPos, Util::Color(255, 0, 0)));

    // --- E. 渲染與退出偵測 ---
    m_Root->Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}

// 碰撞判定函式
bool App::IsColliding(const std::shared_ptr<Util::GameObject>& player, const std::shared_ptr<Util::GameObject>& stone) {
    glm::vec2 pPos = player->m_Transform.translation;
    glm::vec2 pSize = player->GetScaledSize();
    float pW = pSize.x * 0.5f; // 縮小判定寬度避免卡死
    float pH = pSize.y;

    glm::vec2 sPos = stone->m_Transform.translation;
    glm::vec2 sSize = stone->GetScaledSize();

    return (pPos.x - pW/2 < sPos.x + sSize.x/2 &&
            pPos.x + pW/2 > sPos.x - sSize.x/2 &&
            pPos.y - pH/2 < sPos.y + sSize.y/2 &&
            pPos.y + pH/2 > sPos.y - sSize.y/2);
}