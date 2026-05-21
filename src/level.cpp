#include "App.hpp"
#include "Util/Image.hpp"
#include "slope.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>

void App::LoadLevel(int level) {
    ClearLevel(); // 1. 先清理上一關的殘留物件
    m_CurrentLevelNum = level;
    m_Score = 0;
    srand(time(NULL));
    // 判斷是否為要載入地圖檔的關卡
    if (level == 1 || level == 2) {
        // --- 2. 定義地圖參數 ---
        const float TILE_SIZE = 23.0f;
        const int MAP_WIDTH = 39;
        const int MAP_HEIGHT = 29;
        std::string mapPath = "../resources/map/level" + std::to_string(level) + ".txt";

        // 計算左上角起始座標 (視窗 897*667，格子 23*23)
        // 寬度中心偏移: -897/2 + 23/2 = -437
        // 高度中心偏移:  667/2 - 23/2 =  322
        float startX = -437.0f;
        float startY = 322.0f;

        // --- 3. 開啟並讀取檔案 ---
        std::ifstream file(mapPath);
        if (!file.is_open()) {
            printf("Failed to open map file: %s\n", mapPath.c_str());
            return;
        }
        std::string line;
        // 逐行讀取 Y 軸 (Row)
        for (int row = 0; row < MAP_HEIGHT && std::getline(file, line); ++row) {
            std::stringstream ss(line);
            std::string cell;

            // 逐格讀取 X 軸 (Column)
            for (int col = 0; col < MAP_WIDTH && ss >> cell; ++col) {
                // 計算當前格子的實際座標
                float posX = startX + (col * TILE_SIZE);
                float posY = startY - (row * TILE_SIZE);
                printf("Loading cell at row %d, col %d: %s (posX: %.2f, posY: %.2f)\n", row, col, cell.c_str(), posX, posY);
                // --- 4. 根據代碼生成物件 ---
                if (cell == "1") {
                    // 隨機產生 1 到 5 的數字
                    int randomIdx = (std::rand() % 4) + 1;
                    std::string stonePath = PIC_PATH + "stone" + std::to_string(randomIdx) + ".png";

                    auto stone = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(stonePath), -1.0f
                    );
                    stone->m_Transform.translation = { posX, posY };
                    m_Stones.push_back(stone);
                    m_Root->AddChild(stone);
                }
                else if (cell == "2") {
                    auto stone = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(PIC_PATH + "trapbottom.png"), -1.0f
                    );
                    stone->m_Transform.translation = { posX, posY - 7.5f};
                    m_Stones.push_back(stone);
                    m_Root->AddChild(stone);
                    // 生成冰陷阱
                    auto trap = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(PIC_PATH + "trap1.png"), -1.0f
                    );
                    trap->m_Transform.translation = { posX, posY + 4.0f };
                    m_IceTraps.push_back(trap);
                    m_Root->AddChild(trap);
                }
                else if (cell == "3") {
                    auto stone = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(PIC_PATH + "trapbottom.png"), -1.0f
                    );
                    stone->m_Transform.translation = { posX, posY - 7.5f};
                    m_Stones.push_back(stone);
                    m_Root->AddChild(stone);
                    // 生成火陷阱
                    auto trap = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(PIC_PATH + "trap2.png"), -1.0f
                    );
                    trap->m_Transform.translation = { posX, posY };
                    m_FireTraps.push_back(trap);
                    m_Root->AddChild(trap);
                }
                else if (cell == "4") {
                    auto stone = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(PIC_PATH + "trapbottom.png"), -1.0f
                    );
                    stone->m_Transform.translation = { posX, posY - 7.5f};
                    m_Stones.push_back(stone);
                    m_Root->AddChild(stone);
                    // 生成陷阱
                    auto trap = std::make_shared<Util::GameObject>(
                        std::make_shared<Util::Image>(PIC_PATH + "trap.png"), -1.0f
                    );
                    trap->m_Transform.translation = { posX, posY };
                    m_Traps.push_back(trap);
                    m_Root->AddChild(trap);
                }
            }
        }
        file.close(); // 讀取完畢關閉串流

        // --- 2. 角色位置重置 ---
        // 稍微向右移，避免貼死左邊緣 (-448.5)
        m_Ice->m_Transform.translation = { -350.0f, -190.0f };
        m_Fire->m_Transform.translation = { -350.0f, -280.0f };
        m_IceVelocityY = 0;
        m_FireVelocityY = 0;

        // --- 3. 門與機關 ---
        // 門的位置原本在 -200, -100，在新視窗中依然適用，但 Y 軸可配合地圖微調
        m_IceDoor->m_Transform.translation = { 370.0f, 240.0f };
        m_FireDoor->m_Transform.translation = { 300.0f, 240.0f };

        m_IceDoorFrameIndex = 0;
        m_FireDoorFrameIndex = 0;
        m_IceDoor->SetDrawable(std::make_shared<Util::Image>(m_IceDoorFrames[0]));
        m_FireDoor->SetDrawable(std::make_shared<Util::Image>(m_FireDoorFrames[0]));

        if (!m_Box) {
            m_Box = std::make_shared<Util::GameObject>(
                std::make_shared<Util::Image>(PIC_PATH + "box.png"), 0.1f
            );
            m_Root->AddChild(m_Box);
        }
        m_BoxVelocityY = 0.0f;
        m_BoxOnGround = false;
        m_Box->m_Transform.translation = { 10.0f, 150.0f };

        // --- 3. 按鈕與機關 (Buttons) ---
        // 按鈕 1
        auto btn1 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.5f);
        btn1->m_Transform.translation = { -300.0f, -10.0f };
        m_Buttons.push_back(btn1);
        m_Root->AddChild(btn1);

        // 按鈕 2
        auto btn2 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "button1.png"), -1.0f);
        btn2->m_Transform.translation = { 100.0f, 130.0f };
        m_Buttons.push_back(btn2);
        m_Root->AddChild(btn2);


        // --- 4. 齒輪/移動地板 (Gears) ---
        // 齒輪 1
        auto gear1 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear1.png"), -1.0f);
        gear1->m_Transform.translation = { 380.0f, 46.0f };
        m_Gears.push_back(gear1);
        m_GearOriginalPositions.push_back(gear1->m_Transform.translation);
        m_Root->AddChild(gear1);

        // 齒輪 2 (垂直旋轉的)
        auto gear2 = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "gear2.png"), -1.0f);
        gear2->m_Transform.translation = { -390.0f, -27.0f };
        m_Gears.push_back(gear2);
        m_GearOriginalPositions.push_back(gear2->m_Transform.translation);
        m_Root->AddChild(gear2);


        // --- 鐵鍊旋轉平台 ---
        if(level == 1) {
            m_ChainPlatform = std::make_shared<ChainPlatform>(
                PIC_PATH + "Lift(1).png",   // 鐵鍊圖片
                PIC_PATH + "Lift(2).png",   // 平台圖片

                glm::vec2(150.0f, -100.0f),    // 鐵鍊位置
                glm::vec2(150.0f, -140.0f),     // 平台位置

                glm::vec2(0.3f, 0.3f),      // 鐵鍊縮放
                glm::vec2(0.3f, 0.3f),      // 平台縮放

                160.0f,                     // 平台碰撞寬度
                12.0f                       // 平台碰撞高度，先用 12 比較貼圖
            );

            m_ChainPlatform->SetRotation(0.0f);

            m_Root->AddChild(m_ChainPlatform->GetChainObject());
            m_Root->AddChild(m_ChainPlatform->GetBoardObject());
        }

        
        // --- 5. 拉桿 (Switches) ---
        auto sw = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(PIC_PATH + "switch1_1.png"), -1.5f);
        sw->m_Transform.translation = { -150.0f, -130.0f };
        m_Switches.push_back(sw);
        m_SwitchStates.push_back(false); // 初始狀態設為關閉
        m_Root->AddChild(sw);

        // // ==========風扇=======================================================

        // std::vector<std::string> fanFrames;
        // std::vector<std::string> windFrames;

        // for (int i = 1; i <= 4; i++) {
        //     fanFrames.push_back(PIC_PATH + "fan(" + std::to_string(i) + ").png");
        // }
        // for (int i = 1; i <= 10; i++) {
        //     windFrames.push_back(PIC_PATH + "wind(" + std::to_string(i) + ").png");
        // }

        // m_Fan = std::make_shared<Fan>(fanFrames, windFrames);
        // m_Root->AddChild(m_Fan->GetWindObject());
        // m_Root->AddChild(m_Fan->GetFanObject());
        // m_Fan->SetActive(true);

        // m_Fan->SetPosition(
        //     glm::vec2(150.0f, -300.0f),
        //     glm::vec2(0.0f, 120.0f)
        // );

        // m_Fan->SetScale(
        //     glm::vec2(0.45f, 0.45f),
        //     glm::vec2(0.45f, 0.85f)
        // );
        // // =====================================================================

        InitDiamonds();
        // --- 5. 斜坡 ---
        //AddSlope(PIC_PATH + "l_tri.png", {500.0f, -179.0f}, {1.2f, 1.2f}, {-14.0f, -14.0f}, {14.0f, 14.0f}, 0.2f, 0.2f, true);
        //AddSlope(PIC_PATH + "r_tri.png", {409.0f, -143.0f}, {1.2f, 1.2f}, {-14.0f, 14.0f}, {14.0f, -14.0f}, 0.2f, 0.1f, true);

    }
}

void App::ClearLevel() {
    // 1. 清理石頭
    for (auto& stone : m_Stones) m_Root->RemoveChild(stone);
    m_Stones.clear();

    // 2. 清理陷阱群 (Traps, IceTraps, FireTraps)
    for (auto& t : m_Traps) m_Root->RemoveChild(t);
    m_Traps.clear();
    for (auto& t : m_IceTraps) m_Root->RemoveChild(t);
    m_IceTraps.clear();
    for (auto& t : m_FireTraps) m_Root->RemoveChild(t);
    m_FireTraps.clear();

    // 3. 清理機關類向量 (Buttons, Gears, Switches)
    // 這是你提到的重點：將原本單一的 cleanup 改為迴圈清理向量
    for (auto& btn : m_Buttons) m_Root->RemoveChild(btn);
    m_Buttons.clear();

    for (auto& gear : m_Gears) m_Root->RemoveChild(gear);
    m_Gears.clear();
    m_GearOriginalPositions.clear(); // 座標向量也要清空

    for (auto& sw : m_Switches) m_Root->RemoveChild(sw);
    m_Switches.clear();
    m_SwitchStates.clear(); // 清空開關狀態向量

    // 4. 清理其餘單一物件 (鑽石等)
    auto cleanup = [&](std::shared_ptr<Util::GameObject>& obj) {
        if (obj) {
            m_Root->RemoveChild(obj);
            obj = nullptr;
        }
    };
    cleanup(m_RedDiamond);
    cleanup(m_BlueDiamond);

    if (m_ChainPlatform) {
        m_Root->RemoveChild(m_ChainPlatform->GetChainObject());
        m_Root->RemoveChild(m_ChainPlatform->GetBoardObject());
        m_ChainPlatform = nullptr;
    }

    // 5. 清理斜坡
    for (auto& slope : m_Slopes) {
        if (slope.GetImage()) m_Root->RemoveChild(slope.GetImage());
    }
    m_Slopes.clear();

    // 6. 重置角色狀態與物理參數
    m_IceVelocityY = 0;
    m_FireVelocityY = 0;
    // 如果你有一個全域的開關總變數也可以重置，但主要應該是清空 m_SwitchStates
}