#include <Geode/Geode.hpp>
using namespace geode::prelude;

class JumpCounterNode : public CCNode {
protected:
    CCLabelBMFont* m_countLabel  = nullptr;
    CCLabelBMFont* m_pbLabel     = nullptr;
    int            m_jumps       = 0;
    int            m_pb          = 0;

    bool init(int personalBest) {
        if (!CCNode::init()) return false;

        m_pb = personalBest;

        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({ 140.f, 46.f });
        bg->setOpacity(140);
        bg->setColor({ 0, 0, 0 });
        bg->setPosition({ 70.f, 23.f });
        this->addChild(bg, 0);

        m_countLabel = CCLabelBMFont::create("0", "bigFont.fnt");
        m_countLabel->setScale(0.55f);
        m_countLabel->setPosition({ 70.f, 30.f });
        this->addChild(m_countLabel, 1);

        m_pbLabel = CCLabelBMFont::create("PB: 0", "goldFont.fnt");
        m_pbLabel->setScale(0.32f);
        m_pbLabel->setPosition({ 70.f, 12.f });
        this->addChild(m_pbLabel, 1);

        this->setContentSize({ 140.f, 46.f });
        refreshDisplay();
        return true;
    }

public:
    static JumpCounterNode* create(int personalBest) {
        auto ret = new JumpCounterNode();
        if (ret->initWithPersonalBest(personalBest)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool initWithPersonalBest(int pb) { return init(pb); }

    void addJump() {
        ++m_jumps;
        refreshDisplay();
    }

    int getJumps() const { return m_jumps; }

    bool finalizePB() {
        if (m_jumps > m_pb) {
            m_pb = m_jumps;
            return true;
        }
        return false;
    }

    int getPB() const { return m_pb; }

private:
    void refreshDisplay() {
        ccColor3B color = { 255, 255, 255 };
        if (m_pb > 0) {
            float ratio = static_cast<float>(m_jumps) / static_cast<float>(m_pb);
            if (ratio >= 1.0f) {
                color = { 255, 80, 80 };
            } else if (ratio >= 0.8f) {
                color = { 255, 220, 60 };
            }
        }

        auto text = std::to_string(m_jumps);
        m_countLabel->setString(text.c_str());
        m_countLabel->setColor(color);

        auto pbText = "PB: " + std::to_string(m_pb);
        m_pbLabel->setString(pbText.c_str());
    }
};

class $modify(JCPlayLayer, PlayLayer) {
    struct Fields {
        JumpCounterNode* m_hud = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (!Mod::get()->getSettingValue<bool>("enabled")) return true;

        int savedPB = Mod::get()->getSavedValue<int64_t>(
            "pb-" + std::to_string(level->m_levelID.value()), 0
        );

        auto hud = JumpCounterNode::create(static_cast<int>(savedPB));
        if (!hud) return true;

        m_fields->m_hud = hud;

        CCSize screen = CCDirector::sharedDirector()->getWinSize();
        hud->setPosition({ 8.f, 8.f });

        this->addChild(hud, 100);

        return true;
    }

    void pushButton(int button, bool player1) {
        PlayLayer::pushButton(button, player1);
        if (button == 1 && player1 && m_fields->m_hud) {
            m_fields->m_hud->addJump();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        savePB();
    }


    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        savePB();
    }

private:
    void savePB() {
        auto hud = m_fields->m_hud;
        if (!hud) return;

        bool isNew = hud->finalizePB();
        if (isNew) {
            int64_t pb = static_cast<int64_t>(hud->getPB());
            auto key = "pb-" + std::to_string(m_level->m_levelID.value());
            Mod::get()->setSavedValue<int64_t>(key, pb);
            log::info("[JumpCounter] New PB: {} jumps", pb);
        }
    }
};
