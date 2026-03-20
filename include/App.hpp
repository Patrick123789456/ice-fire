#ifndef APP_HPP
#define APP_HPP


#include "pch.hpp"
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include <vector>
#include <string> 

// 圖片與字型路徑基底
const std::string PIC_PATH = "../Resources/picture/";
const std::string FONT_PATH = "../Resources/font/";

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

private:
    // AABB 碰撞偵測輔助函式
    bool IsColliding(const std::shared_ptr<Util::GameObject>& player, const std::shared_ptr<Util::GameObject>& stone);

private:
    State m_CurrentState = State::START;

    std::shared_ptr<Util::Renderer> m_Root;
    
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_Ice;
    std::shared_ptr<Util::GameObject> m_Fire;

    // 石頭平台列表
    std::vector<std::shared_ptr<Util::GameObject>> m_Stones;

    // --- 新增：座標顯示 Text ---
    std::shared_ptr<Util::GameObject> m_IcePosText; // 左上
    std::shared_ptr<Util::GameObject> m_FirePosText; // 右上

    // 物理參數
    const float m_Gravity = 0.4f;      // 重力調小了 (原為 0.8)
    const float m_JumpForce = 12.0f;   // 跳躍初速
    const float m_MoveSpeed = 5.0f;    // 左右移動速度

    float m_IceVelocityY = 0.0f;
    float m_FireVelocityY = 0.0f;

    // 紀錄是否在地面上，防止二段跳
    bool m_IceOnGround = false;
    bool m_FireOnGround = false;
};

#endif