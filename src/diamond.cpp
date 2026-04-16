#include "App.hpp"

void App::InitDiamonds() {
    // ===== 紅寶石 =====
    m_RedDiamond = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "r_diamond.png"), -1.0f
    );
    m_RedDiamond->m_Transform.translation = { -150.0f, -50.0f };
    m_RedDiamond->m_Transform.scale = { 0.6f, 0.6f };
    m_RedDiamondBasePos = m_RedDiamond->m_Transform.translation;
    m_RedDiamondCollected = false;
    m_Root->AddChild(m_RedDiamond);

    // ===== 藍寶石 =====
    m_BlueDiamond = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(PIC_PATH + "b_diamond.png"), -1.0f
    );
    m_BlueDiamond->m_Transform.translation = { 150.0f, -50.0f };
    m_BlueDiamond->m_Transform.scale = { 0.6f, 0.6f };
    m_BlueDiamondBasePos = m_BlueDiamond->m_Transform.translation;
    m_BlueDiamondCollected = false;
    m_Root->AddChild(m_BlueDiamond);
}

void App::UpdateDiamonds() {
    m_DiamondFloatTime += m_DiamondFloatSpeed;
    float offsetY = std::sin(m_DiamondFloatTime) * m_DiamondFloatRange;

    if (m_RedDiamond && !m_RedDiamondCollected) {
        m_RedDiamond->m_Transform.translation.y = m_RedDiamondBasePos.y + offsetY;
    }

    if (m_BlueDiamond && !m_BlueDiamondCollected) {
        m_BlueDiamond->m_Transform.translation.y = m_BlueDiamondBasePos.y + offsetY;
    }
}

void App::CheckDiamondCollection() {
    // Fire 吃紅寶石
    if (m_RedDiamond && !m_RedDiamondCollected) {
        if (IsColliding(m_Fire, m_RedDiamond)) {
            m_RedDiamondCollected = true;
            m_Root->RemoveChild(m_RedDiamond);
            m_RedDiamond.reset();
            m_Score += 1;
        }
    }

    // Ice 吃藍寶石
    if (m_BlueDiamond && !m_BlueDiamondCollected) {
        if (IsColliding(m_Ice, m_BlueDiamond)) {
            m_BlueDiamondCollected = true;
            m_Root->RemoveChild(m_BlueDiamond);
            m_BlueDiamond.reset();
            m_Score += 1;
        }
    }
}