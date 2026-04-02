#include <Geode/Geode.hpp>
#include <algorithm>
#include <alphalaneous.level-storage-api/include/LevelStorageAPI.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;
using namespace std;

void applyObjectSettings() {
	LevelEditorLayer* editorLayer = LevelEditorLayer::get();
	if (!editorLayer) {
		log::error("Couldn't get editor layer, not applying object settings");
		return;
	}
	CCArray* objects = editorLayer->getAllObjects();

	bool dontEnter = alpha::level_storage::getSavedValue<bool>(editorLayer, "dont-enter-toggle"_spr);
	bool dontFade = alpha::level_storage::getSavedValue<bool>(editorLayer, "dont-fade-toggle"_spr);
	bool noGlow = alpha::level_storage::getSavedValue<bool>(editorLayer, "disable-glow-toggle"_spr);

	for (auto* obj : CCArrayExt<GameObject*>(objects)) {
		if (dontEnter) {
			obj->m_isDontEnter = true;
		}
		if (dontFade) {
			obj->m_isDontFade = true;
		}
		if (noGlow) {
			obj->m_hasNoGlow = true;
		}
	}
}

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

		this->m_mainLayer->addChild(settingsLayout);

		settingsLayout->addChild(createSetting(settingsLayout, "Disable Glow", "disable-glow"));
		settingsLayout->addChild(createSetting(settingsLayout, "Dont Fade", "dont-fade"));
		settingsLayout->addChild(createSetting(settingsLayout, "Dont Enter", "dont-enter"));
		settingsLayout->addChild(createSetting(settingsLayout, "Apply on Save", "apply-on-save"));

		auto applyMenu = CCMenu::create();
		applyMenu->setID("apply-menu"_spr);
		applyMenu->setPosition({0.0f, 0.0f});
		this->m_mainLayer->addChild(applyMenu);

		auto applyButton = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("Apply"),
			this,
			menu_selector(SettingsPopup::applySettings)
		);
		applyButton->setID("apply-button"_spr);
		applyButton->setPosition({120.f, 0.0f});
		
		applyMenu->addChild(applyButton);
		settingsLayout->updateLayout();

        return true;
    }
	void applySettings(CCObject* sender) {
		applyObjectSettings();
		onClose(this);
	}
	void onToggle(CCObject* sender) {
		LevelEditorLayer* editorLayer = LevelEditorLayer::get();
		if (!editorLayer)
		{
			log::error("Couldn't get editor layer, not saving setting");
			return;
		}

        auto toggle = static_cast<CCMenuItemToggler*>(sender);
        bool isOn = !toggle->isToggled();
		
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

		LevelEditorLayer* editorLayer = LevelEditorLayer::get();
		if (!editorLayer)
		{
			log::error("Couldn't get editor layer, not saving setting");
			return nullptr;
		}

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
		if (!guidelinesMenu) {
			log::error("Couldn't find guidelines menu, not adding layout settings button");
			return true;
		}
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
		if (alpha::level_storage::getSavedValue<bool>(this->m_editorLayer, "apply-on-save-toggle"_spr)) {
			applyObjectSettings();
		}
		EditorPauseLayer::saveLevel();
	}
};