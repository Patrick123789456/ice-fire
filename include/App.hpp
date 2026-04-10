#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include <vector>
#include <string>
#include <memory>
#include "Slope.h"

class App {
public:
    enum class State {
        START,
        UPDATE,
        DEAD,
        PAUSE,
        END
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

private:
    //slope
    std::vector<Slope> m_Slopes;

    float m_FootOffset = 32.0f;      // 角色中心到腳底距離，要自己微調
    float m_SlopeTolerance = 10.0f;  // 站在斜坡上的容許誤差
    float m_SlopeSnapHeight = 16.0f;   // 只吸附離坡面不遠的角色

    // 碰撞偵測：player 碰撞 stone 或 trap
    bool IsColliding(const std::shared_ptr<Util::GameObject>& player, const std::shared_ptr<Util::GameObject>& target);
    // 關卡管理
    int m_CurrentLevelNum = 1;
    void LoadLevel(int level);
    void ClearLevel();
    State m_CurrentState = State::START;

    std::shared_ptr<Util::Renderer> m_Root;
    std::shared_ptr<Util::GameObject> m_PauseScreen;
    std::shared_ptr<Util::GameObject> m_DeadScreen;
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_Ice;
    std::shared_ptr<Util::GameObject> m_Fire;
    std::shared_ptr<Util::GameObject> m_Trap;
    std::shared_ptr<Util::GameObject> m_IceTrap;  // Ice 碰到會死 (Trap1)
    std::shared_ptr<Util::GameObject> m_FireTrap; // Fire 碰到會死 (Trap2)
    std::vector<std::shared_ptr<Util::GameObject>> m_Stones;
    std::shared_ptr<Util::GameObject> m_IceDoor;
    std::shared_ptr<Util::GameObject> m_FireDoor;
    std::shared_ptr<Util::GameObject> m_Box;                //箱子

    // ===== 新增：門動畫 =====
    std::vector<std::string> m_IceDoorFrames;
    std::vector<std::string> m_FireDoorFrames;

    int m_IceDoorFrameIndex = 0;
    int m_FireDoorFrameIndex = 0;

    bool m_IceDoorOpening = false;
    bool m_FireDoorOpening = false;

    int m_DoorAnimCounter = 0;
    int m_DoorAnimSpeed = 2;   // 數字越大，門動畫越慢

    // ===== 新增：紅色寶石 =====
    std::shared_ptr<Util::GameObject> m_RedDiamond;
    bool m_RedDiamondCollected = false;
    std::shared_ptr<Util::GameObject> m_BlueDiamond;
    bool m_BlueDiamondCollected = false;

    glm::vec2 m_RedDiamondBasePos;
    glm::vec2 m_BlueDiamondBasePos;

    float m_DiamondFloatTime = 0.0f;
    float m_DiamondFloatSpeed = 0.02f;   // 飄浮速度
    float m_DiamondFloatRange = 4.0f;    // 上下飄動幅度

    // ===== 新增：分數 =====
    int m_Score = 0;
    std::shared_ptr<Util::GameObject> m_ScoreText;


    // 座標文字
    std::shared_ptr<Util::GameObject> m_IcePosText;
    std::shared_ptr<Util::GameObject> m_FirePosText;


    //機關
    std::shared_ptr<Util::GameObject> m_Button;
    std::shared_ptr<Util::GameObject> m_Gear;
    glm::vec2 m_GearOriginalPos;
    std::shared_ptr<Util::GameObject> m_Switch;
    std::shared_ptr<Util::GameObject> m_Gear2;
    glm::vec2 m_Gear2OriginalPos;
    bool m_IsSwitchOn = false;  // 開關狀態

    float m_Gravity = 0.4f;
    float m_JumpForce = 12.0f;
    float m_MoveSpeed = 3.0f;
    float m_BoxMoveSpeedLimit = 3.0f;

    float m_IceVelocityY = 0.0f;
    float m_FireVelocityY = 0.0f;

    float m_BoxVelocityY = 0.0f; // 箱子的垂直速度
    bool m_BoxOnGround = false;  // 箱子是否在地板上

    bool m_IceOnGround = false;
    bool m_FireOnGround = false;

    void AddSlope(const std::string& imagePath,
              const glm::vec2& imagePos,
              const glm::vec2& imageScale,
              const glm::vec2& localStart,
              const glm::vec2& localEnd,
              float slideSpeed,
              float moveFactor,
              bool isSolid = true,
              float zIndex = -1.0f);

    void ApplySlopeToPlayer(const std::shared_ptr<Util::GameObject>& player,
                            float& velocityY,
                            bool& onGround,
                            float& dx);
};

#endif