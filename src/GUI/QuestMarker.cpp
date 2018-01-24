#include "GUI/QuestMarker.h"
#include "ResourceManager.h"
#include "GlobalResource.h"
#include "CharacterCore.h"
#include "GameObjectComponents/TooltipWindowComponent.h"

const int QuestMarker::SIZE = 16;

QuestMarker::QuestMarker(const QuestData& questData, CharacterCore* core) {
	m_questData = questData;
	m_characterCore = core;
	m_isInputInDefaultView = true;
	
	setBoundingBox(sf::FloatRect(0.f, 0.f, static_cast<float>(SIZE), static_cast<float>(SIZE)));
	m_sprite.setTexture(*g_resourceManager->getTexture(GlobalResource::TEX_GUI_QUESTMARKERS));

	m_tooltipComponent = new TooltipWindowComponent("", this);
	m_tooltipComponent->setWindowOffset(sf::Vector2f(0.f, 20.f));
	addComponent(m_tooltipComponent);
}

void QuestMarker::render(sf::RenderTarget& renderTarget) {
	GameObject::render(renderTarget);
	renderTarget.draw(m_sprite);
}

void QuestMarker::setPosition(const sf::Vector2f& pos) {
	GameObject::setPosition(pos);
	m_sprite.setPosition(pos);
}

bool QuestMarker::isActive() const {
	return m_isActive;
}

void QuestMarker::setActive(bool active) {
	m_isActive = active;
	int y = m_questData.isMainQuest ? 1 : 0;
	int x;

	if (!m_isActive) {
		x = 0;
	}
	else if (m_characterCore->isQuestComplete(m_questData.id)) {
		x = 3;
	}
	else {
		x = 2;
	}
	
	m_sprite.setTextureRect(sf::IntRect(x * SIZE, y * SIZE, SIZE, SIZE));
}