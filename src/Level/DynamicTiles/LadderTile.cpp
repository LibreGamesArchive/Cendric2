#include "Level/DynamicTiles/LadderTile.h"
#include "Spells/Spell.h"
#include "Level/LevelMainCharacter.h"
#include "Registrar.h"

REGISTER_LEVEL_DYNAMIC_TILE(LevelDynamicTileID::Ladder, LadderTile)

const int LadderTile::LADDER_STEP = 15;

bool LadderTile::init(const LevelTileProperties& properties) {
	if (!contains(properties, std::string("size"))) return false;
	m_size = std::stoi(properties.at(std::string("size")));
	if (m_size < 2 || m_size > 100) return false;

	setSpriteOffset(sf::Vector2f(-10.f, -TILE_SIZE_F));
	setPositionOffset(sf::Vector2f(10.f, 0.f));
	setBoundingBox(sf::FloatRect(0.f, 0.f, TILE_SIZE_F - 20.f, TILE_SIZE_F * m_size));

	return true;
}

void LadderTile::loadAnimation(int skinNr) {
	const sf::Texture* tex = g_resourceManager->getTexture(getSpritePath());
	
	sf::Sprite sprite;
	sprite.setTexture(*tex);

	int length = std::max(1, m_size);
	length++;
	int tilepart = 2 - length % 3;

	sprite.setTextureRect(sf::IntRect(skinNr * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));
	m_sprites.push_back(sprite);
	length--;

	while (length > 1) {
		sprite.setTextureRect(sf::IntRect(skinNr * TILE_SIZE, TILE_SIZE + tilepart * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		m_sprites.push_back(sprite);

		tilepart = (tilepart + 1) % 3;
		length--;
	}

	sprite.setTextureRect(sf::IntRect(skinNr * TILE_SIZE, 4 * TILE_SIZE, TILE_SIZE, TILE_SIZE));
	m_sprites.push_back(sprite);

	// load arrow
	const sf::Texture* texture = g_resourceManager->getTexture(GlobalResource::TEX_GUI_LADDER_ARROW);
	m_arrow.setTexture(*texture);
}

void LadderTile::update(const sf::Time& frametime) {
	m_showSprite = false;
	m_time += frametime;
	LevelDynamicTile::update(frametime);
}

void LadderTile::render(sf::RenderTarget& target) {
	if (m_isFirstRenderIteration) {
		for (auto& sprite : m_sprites) {
			target.draw(sprite);
		}
		m_isFirstRenderIteration = false;
	}
	else {
		m_isFirstRenderIteration = true;
	}
}

void LadderTile::renderAfterForeground(sf::RenderTarget& target) {
	LevelDynamicTile::renderAfterForeground(target);
	if (m_showSprite) target.draw(m_arrow);
}

void LadderTile::onHit(LevelMovableGameObject* mob) {
	if (mob->getConfiguredType() != _LevelMainCharacter)  return;
	if (mob->getState() == GameObjectState::Climbing_1 || mob->getState() == GameObjectState::Climbing_2) return;
	m_showSprite = true;

	float variance = 4.f;
	float speed = 6.f;
	float offset = 60.f + variance * std::cos(speed * m_time.asSeconds());
	
	float width = static_cast<float>(m_arrow.getTextureRect().width);
	m_arrow.setPosition(m_mainChar->getPosition().x + 0.5f * (m_mainChar->getSize().x - width), m_mainChar->getPosition().y - offset);
}

void LadderTile::setPosition(const sf::Vector2f& position) {
	GameObject::setPosition(position);
	float offset = 0.f;
	for (auto& sprite : m_sprites) {
		sprite.setPosition(getPosition().x + getSpriteOffset().x, getPosition().y + getSpriteOffset().y + offset);
		offset += TILE_SIZE;
	}
}

float LadderTile::getClimbingPositionY(GameObject* object) const {
	float goHeight = object->getBoundingBox()->height;
	float ladderBottom = getPosition().y + getBoundingBox()->height;
	float goBottom = std::round(object->getPosition().y + goHeight);

	int goDiff = static_cast<int>(std::round(ladderBottom - goBottom));
	goDiff = goDiff % LADDER_STEP;
	int sign = 1; // go down

	// check for nearest direction
	if (static_cast<float>(goDiff) > LADDER_STEP / 2.f) {
		goDiff = LADDER_STEP - goDiff;
		sign = -1;
	}

	float newBottom = goBottom + sign * goDiff;
	if (newBottom - goHeight > ladderBottom - 1) {
		newBottom -= LADDER_STEP;
	}
	else if (newBottom < getPosition().y + 1) {
		newBottom += LADDER_STEP;
	}

	return newBottom - goHeight;
}

std::string LadderTile::getSpritePath() const {
	return "res/texture/level_dynamic_tiles/spritesheet_tiles_ladder.png";
}