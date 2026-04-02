#include <Geode/Geode.hpp>
#include <algorithm>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;
using namespace std;

class SettingsPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(240.f, 160.f))
            return false;

        this->m_noElasticity = true;
        this->setTitle("Layout Settings");

		auto settingsLayout = CCMenu::create();
		settingsLayout->setScale(0.4f);
		settingsLayout->setPosition({140.f, 70.f});
		settingsLayout->setLayout(ColumnLayout::create()->setAxisReverse(true));

		this->getChildByIndex(0)->addChild(settingsLayout);

		settingsLayout->addChild(createSetting(settingsLayout, "Disable Glow", "disable-glow"));
		settingsLayout->addChild(createSetting(settingsLayout, "Dont Fade", "dont-fade"));
		settingsLayout->addChild(createSetting(settingsLayout, "Dont Enter", "dont-enter"));
		settingsLayout->updateLayout();

        return true;
    }
	
	void onToggle(CCObject* sender) {
        auto toggle = static_cast<CCMenuItemToggler*>(sender);
        bool isOn = !toggle->isToggled();

		LevelEditorLayer* editorLayer = CCDirector::get()->getRunningScene()->getChildByType<LevelEditorLayer>(0);
		alpha::level_storage::setSavedValue(editorLayer, toggle->getID(), isOn);
    }
	CCNode* createSetting(CCNode* parent, char const* text, std::string id) {
		auto menu = CCMenu::create();
		menu->setLayout(RowLayout::create()->setAutoScale(false)->setAxisAlignment(AxisAlignment::Start));

		auto textLabel = CCLabelBMFont::create(text, "bigFont.fnt");
		textLabel->setScale(1.5f);
		auto toggle = CCMenuItemToggler::createWithStandardSprites(
			this,
			menu_selector(SettingsPopup::onToggle),
			1.25f
		);

		auto toggleID = ""_spr + id + "-toggle";

		LevelEditorLayer* editorLayer = CCDirector::get()->getRunningScene()->getChildByType<LevelEditorLayer>(0);
		if (alpha::level_storage::getSavedValue<bool>(editorLayer, toggleID)) {
			toggle->toggle(alpha::level_storage::getSavedValue<bool>(editorLayer, toggleID));
		}
		else {
			alpha::level_storage::setSavedValue(editorLayer, toggleID, false);
			toggle->toggle(false);
		}
		toggle->setID(toggleID);
		textLabel->setID(""_spr + id + "-label");
		menu->addChild(toggle);
		menu->addChild(textLabel);
		menu->updateLayout();

		return menu;
	}

public:
    static SettingsPopup* create() {
        auto ret = new SettingsPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

#include <Geode/modify/EditorPauseLayer.hpp>
class $modify(MyEditorPauseLayer, EditorPauseLayer) {
	bool init(LevelEditorLayer* layer) {
		if (!EditorPauseLayer::init(layer)) {
			return false;
		}

		auto button = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::createWithSprite("regularBlock.png"_spr, 0.75f),
			this,
			menu_selector(MyEditorPauseLayer::openSettings)
		);
		button->setScale(0.8f);
		button->setID("layout-settings"_spr);

		CCNode* guidelinesMenu = this->getChildByID("guidelines-menu");
		guidelinesMenu->addChild(button);
		guidelinesMenu->updateLayout();

		return true;
	}

	void openSettings(CCObject*) {
		SettingsPopup::create()->show();
	}
};

#include <Geode/modify/EditorPauseLayer.hpp>
class $modify(EditorPauseLayer) {
	void saveLevel() {
		CCArray* objects = m_editorLayer->getAllObjects();
		for (auto* obj : CCArrayExt<GameObject*>(objects)) {
			if (alpha::level_storage::getSavedValue<bool>(m_editorLayer, "dont-enter-toggle"_spr)) {
				obj->m_isDontEnter = true;
			}
			if (alpha::level_storage::getSavedValue<bool>(m_editorLayer, "dont-fade-toggle"_spr)) {
				obj->m_isDontFade = true;
			}
			if (alpha::level_storage::getSavedValue<bool>(m_editorLayer, "disable-glow-toggle"_spr)) {
				obj->m_hasNoGlow = true;
			}
		}

		EditorPauseLayer::saveLevel();
	}
};