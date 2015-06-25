#include "LevelMainCharacter.h"

LevelMainCharacter::LevelMainCharacter(Level* level) : LevelMovableGameObject(level)
{
	load();
	m_keyMap.insert(
	{
		{ Key::SpellChop, SpellID::Chop },
		{ Key::SpellIce, SpellID::Ice },
		{ Key::SpellFire, SpellID::Fire },
		{ Key::SpellForcefield, SpellID::Forcefield }
	});

	// these values should be modified by a staff
	SpellBean fireSpell = DEFAULT_FIRE;
	fireSpell.amount = 3;
	fireSpell.reflectCount = 2;
	SpellBean chopSpell = DEFAULT_CHOP;
	SpellBean forcefieldSpell = DEFAULT_FORCEFIELD;
	SpellBean iceSpell = DEFAULT_ICE;

	m_spellManager->addSpell(chopSpell);
	m_spellManager->addSpell(iceSpell);
	m_spellManager->addSpell(forcefieldSpell);
	m_spellManager->addSpell(fireSpell);
}

LevelMainCharacter::~LevelMainCharacter()
{
	g_resourceManager->deleteResource(ResourceID::Texture_mainChar);
	m_keyMap.clear();
}

void LevelMainCharacter::onHit(Spell* spell)
{
	if (m_state == GameObjectState::Dead)
	{
		return;
	}
	// check for owner
	if (spell->getOwner() == this)
	{
		return;
	}
	int damage = 0;
	switch (spell->getConfiguredSpellID())
	{
	case SpellID::Ice:
		damage = spell->getDamage() - m_attributes->resistanceIce;
		spell->setDisposed();
		break;
	case SpellID::Fire:
		damage = spell->getDamage() - m_attributes->resistanceFire;
		spell->setDisposed();
		break;
	case SpellID::Chop:
		damage = spell->getDamage() - m_attributes->resistancePhysical;
		spell->setDisposed();
		break;
	default:
		return;
	}
	addDamage(damage);
}

void LevelMainCharacter::handleAttackInput()
{
	// update current spell
	for (auto const &it : m_keyMap) {
		if (g_inputController->isKeyActive(it.first))
		{
			m_spellManager->setCurrentSpell(it.second);
		}
	}

	// handle attack input
	if (g_inputController->isMouseJustPressedLeft())
	{
		std::vector<Spell*> holder = m_spellManager->getSpells();

		if (!holder.empty())
		{
			int div = 0;
			int sign = 1;
			for (auto& it : holder)
			{
				it->load(getLevel(), this, g_inputController->getMousePosition(), div * sign);
				m_screen->addObject(GameObjectType::_Spell, it);
				sign = -sign;
				if (sign == -1)
				{
					div += 1;
				}
			}
			if (holder.at(0)->getConfiguredTriggerFightAnimation()) {
				m_fightAnimationTime = sf::milliseconds(5 * 70); // duration of fight animation
			}
		}
	}
}

void LevelMainCharacter::handleMovementInput()
{
	float newAccelerationX = 0;

	if (g_inputController->isKeyActive(Key::Left))
	{
		m_nextIsFacingRight = false;
		newAccelerationX -= getConfiguredWalkAcceleration();
	}
	if (g_inputController->isKeyActive(Key::Right))
	{
		m_nextIsFacingRight = true;
		newAccelerationX += getConfiguredWalkAcceleration();
	}
	if (g_inputController->isKeyActive(Key::Jump) && m_isGrounded)
	{
		setVelocityY(-getConfiguredMaxVelocityY()); // first jump vel will always be max y vel. 
	}

	setAcceleration(sf::Vector2f(newAccelerationX, getConfiguredGravityAcceleration()));
}

void LevelMainCharacter::addDamage(int damage)
{
	if (m_state == GameObjectState::Dead) return;
	m_attributes->currentHealthPoints = std::max(0, std::min(m_attributes->maxHealthPoints, m_attributes->currentHealthPoints - damage));
	if (m_attributes->currentHealthPoints == 0)
	{
		m_isDead = true;
	}
	setSpriteColor(sf::Color::Red, sf::milliseconds(100));
}

void LevelMainCharacter::setDead()
{
	m_attributes->currentHealthPoints = 0;
	m_isDead = true;
}

void LevelMainCharacter::setCharacterCore(CharacterCore* core)
{
	m_core = core;
	m_attributes = core->getTotalAttributes();
	setPosition(core->getData().currentLevelPosition);
}

void LevelMainCharacter::update(const sf::Time& frameTime)
{
	LevelMovableGameObject::update(frameTime);
	if (!m_isDead)
	{
		updateRegeneration(frameTime);
	}
}

void LevelMainCharacter::updateRegeneration(const sf::Time& frameTime)
{
	m_timeSinceRegeneration += frameTime;
	if (m_timeSinceRegeneration >= sf::seconds(1))
	{
		m_timeSinceRegeneration -= sf::seconds(1);
		m_attributes->currentHealthPoints += m_attributes->healthRegenerationPerS;
		m_attributes->currentManaPoints += m_attributes->manaRegenerationPerS;
		if (m_attributes->currentHealthPoints > m_attributes->maxHealthPoints)
		{
			m_attributes->currentHealthPoints = m_attributes->maxHealthPoints;
		}
		if (m_attributes->currentManaPoints > m_attributes->maxManaPoints)
		{
			m_attributes->currentManaPoints = m_attributes->maxManaPoints;
		}
	}
}

void LevelMainCharacter::load()
{
	setBoundingBox(sf::FloatRect(0.f, 0.f, 30.f, 100.f));
	setSpriteOffset(sf::Vector2f(-25.f, -20.f));

	Animation walkingAnimation;
	walkingAnimation.setSpriteSheet(g_resourceManager->getTexture(ResourceID::Texture_mainChar));
	walkingAnimation.addFrame(sf::IntRect(0, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(80, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(160, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(240, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(320, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(400, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(480, 0, 80, 120));
	walkingAnimation.addFrame(sf::IntRect(560, 0, 80, 120));

	addAnimation(GameObjectState::Walking, walkingAnimation);

	Animation idleAnimation;
	idleAnimation.setSpriteSheet(g_resourceManager->getTexture(ResourceID::Texture_mainChar));
	idleAnimation.addFrame(sf::IntRect(640, 0, 80, 120));

	addAnimation(GameObjectState::Idle, idleAnimation);

	Animation jumpingAnimation;
	jumpingAnimation.setSpriteSheet(g_resourceManager->getTexture(ResourceID::Texture_mainChar));
	jumpingAnimation.addFrame(sf::IntRect(720, 0, 80, 120));

	addAnimation(GameObjectState::Jumping, jumpingAnimation);

	Animation fightingAnimation;
	fightingAnimation.setSpriteSheet(g_resourceManager->getTexture(ResourceID::Texture_mainChar));
	fightingAnimation.addFrame(sf::IntRect(800, 0, 80, 120));
	fightingAnimation.addFrame(sf::IntRect(880, 0, 80, 120));
	fightingAnimation.addFrame(sf::IntRect(960, 0, 80, 120));
	fightingAnimation.addFrame(sf::IntRect(1040, 0, 80, 120));
	fightingAnimation.addFrame(sf::IntRect(1040, 0, 80, 120));

	addAnimation(GameObjectState::Fighting, fightingAnimation);

	Animation deadAnimation;
	deadAnimation.setSpriteSheet(g_resourceManager->getTexture(ResourceID::Texture_mainChar));
	deadAnimation.addFrame(sf::IntRect(1120, 0, 80, 120));

	addAnimation(GameObjectState::Dead, deadAnimation);

	setFrameTime(sf::seconds(0.07f));

	// initial values
	m_state = GameObjectState::Idle;
	m_isFacingRight = true;
	setCurrentAnimation(getAnimation(m_state), !m_isFacingRight);
	playCurrentAnimation(true);

	setDebugBoundingBox(sf::Color::White);
}

float LevelMainCharacter::getConfiguredMaxVelocityY() const
{
	return 600.0f;
}

float LevelMainCharacter::getConfiguredMaxVelocityX() const
{
	return 200.0f;
}

float LevelMainCharacter::getConfiguredDampingGroundPersS() const
{
	return 0.999f;
}

GameObjectType LevelMainCharacter::getConfiguredType() const
{
	return GameObjectType::_MainCharacter;
}

void LevelMainCharacter::lootItem(ItemID item, int quantity) const
{
	std::map<ItemID, int>* coreItems = m_core->getItems();
	auto it = coreItems->find(item);

	if (it != coreItems->end())
	{
		coreItems->at(item) = coreItems->at(item) + quantity;
	}
	else
	{
		coreItems->insert({item, quantity});
	}
}

void LevelMainCharacter::lootItems(std::map<ItemID, int>& items) const
{
	for (auto it : items)
	{
		lootItem(it.first, it.second);
	}
}

void LevelMainCharacter::addGold(int gold) const
{
	m_core->addGold(gold);
}
