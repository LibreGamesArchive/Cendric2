#include "Level/LevelMovableGameObject.h"
#include "Spells/SpellManager.h"
#include "Level/Level.h"
#include "Level/MOBBehavior/MovingBehavior.h"
#include "Level/MOBBehavior/AttackingBehavior.h"
#include "Level/DamageNumbers.h"

LevelMovableGameObject::LevelMovableGameObject(const Level* level) : MovableGameObject() {
	m_level = level;
	m_foodAttributes.first = sf::Time::Zero;
	m_registeredSpellCreators = new std::set<SpellCreator*>();
}

LevelMovableGameObject::~LevelMovableGameObject() {
	delete m_spellManager;
	delete m_movingBehavior;
	delete m_attackingBehavior;
	delete m_damageNumbers;
	delete m_registeredSpellCreators;
}

void LevelMovableGameObject::update(const sf::Time& frameTime) {
	// update times
	updateTime(m_stunnedTime, frameTime);
	updateTime(m_fearedTime, frameTime);

	if (m_attackingBehavior && !m_isDead) m_attackingBehavior->update(frameTime);
	if (m_movingBehavior) m_movingBehavior->update(frameTime);

	m_level->collideWithDynamicTiles(this, *getBoundingBox());
	m_spellManager->update(frameTime);

	updateDamageNumbers(frameTime);
	MovableGameObject::update(frameTime);

	if (!m_isDead) {
		updateAttributes(frameTime);
	}
	setAccelerationX(0.f);
}

void LevelMovableGameObject::renderAfterForeground(sf::RenderTarget& target) {
	MovableGameObject::renderAfterForeground(target);
	if (m_damageNumbers) {
		m_damageNumbers->render(target);
	}
}

void LevelMovableGameObject::updateAttributes(const sf::Time& frameTime) {
	// update food attributes
	if (m_foodAttributes.first > sf::Time::Zero) {
		m_foodAttributes.first -= frameTime;
		if (m_foodAttributes.first <= sf::Time::Zero) {
			m_foodAttributes.first = sf::Time::Zero;
			m_attributes.removeBean(m_foodAttributes.second);
		}
	}

	// update buff attributes
	for (size_t i = 0; i < m_buffAttributes.size();/* don't increment here, we remove on the fly */) {
		m_buffAttributes[i].first -= frameTime;
		if (m_buffAttributes[i].first <= sf::Time::Zero) {
			m_attributes.removeBean(m_buffAttributes[i].second);
			m_buffAttributes.erase(m_buffAttributes.begin() + i);
		}
		else {
			i++;
		}
	}

	// health regeneration
	updateHealthRegeneration(frameTime);

	// update debuff attributes
	for (size_t i = 0; i < m_dots.size(); /* don't increment here, we remove on the fly */) {
		int prevSecond = static_cast<int>(std::floor(m_dots[i].duration.asSeconds()));
		m_dots[i].duration -= frameTime;
		int thisSecond = std::max(-1, static_cast<int>(std::floor(m_dots[i].duration.asSeconds())));
		addDamage(m_dots[i].damage * (prevSecond - thisSecond), m_dots[i].damageType, true, false);
		if (m_dots[i].duration <= sf::Time::Zero) {
			m_dots.erase(m_dots.begin() + i);
		}
		else {
			i++;
		}
	}
}

void LevelMovableGameObject::updateDamageNumbers(const sf::Time& frameTime) {
	if (!m_damageNumbers) return;

	updateTime(m_timeSinceDotNumbers, frameTime);
	if (m_timeSinceDotNumbers == sf::Time::Zero) {
		m_timeSinceDotNumbers = sf::seconds(1.f);
		if (m_currentDotDamage > 0) {
			m_damageNumbers->emitNumber(m_currentDotDamage, 
				sf::Vector2f(getPosition().x + 0.5f * getSize().x, getPosition().y),
				DamageNumberType::DamageOverTime, false);
			m_currentDotDamage = 0;
		}
	}
	m_damageNumbers->update(frameTime);
}

void LevelMovableGameObject::updateHealthRegeneration(const sf::Time& frameTime) {
	m_timeSinceRegeneration += frameTime;
	if (m_timeSinceRegeneration >= sf::seconds(1)) {
		m_timeSinceRegeneration -= sf::seconds(1);
		
		if (!(m_isInvincible && m_attributes.healthRegenerationPerS < 0)) {
			m_attributes.currentHealthPoints += m_attributes.healthRegenerationPerS;
		}

		if (m_damageNumbers) {
			const sf::Vector2f& pos = getPosition();
			const sf::Vector2f& size = getSize();
			sf::Vector2f numberPos = sf::Vector2f(pos.x + 0.5f * size.x, pos.y);

			if (m_attributes.healthRegenerationPerS > 0 && m_damageNumbers) {
				m_damageNumbers->emitNumber(m_attributes.healthRegenerationPerS, numberPos, DamageNumberType::HealOverTime, false);
			}
			else if (m_attributes.healthRegenerationPerS < 0) {
				m_damageNumbers->emitNumber(std::abs(m_attributes.healthRegenerationPerS), numberPos, DamageNumberType::DamageOverTime, false);
			}
		}

		if (m_attributes.currentHealthPoints > m_attributes.maxHealthPoints) {
			m_attributes.currentHealthPoints = m_attributes.maxHealthPoints;
		}

		if (m_attributes.currentHealthPoints <= 0) {
			setDead();
		}
	}
}

void LevelMovableGameObject::addAttributes(const sf::Time& duration, const AttributeData& attributes) {
	m_attributes.addBean(attributes);
	m_buffAttributes.push_back(std::pair<sf::Time, AttributeData>(duration, attributes));
}

void LevelMovableGameObject::calculateUnboundedVelocity(const sf::Time& frameTime, sf::Vector2f& nextVel) const {
	if (!m_movingBehavior) return;
	m_movingBehavior->calculateUnboundedVelocity(frameTime, nextVel);
}

void LevelMovableGameObject::addDamage(int damage_, DamageType damageType, bool overTime, bool critical) {
	int damage = 0;
	switch (damageType) {
	case DamageType::Physical:
		damage = static_cast<int>(damage_ * m_attributes.physicalMultiplier);
		break;
	case DamageType::Ice:
		damage = static_cast<int>(damage_ * m_attributes.iceMultiplier);
		break;
	case DamageType::Fire:
		damage = static_cast<int>(damage_ * m_attributes.fireMultiplier);
		break;
	case DamageType::Shadow:
		damage = static_cast<int>(damage_ * m_attributes.shadowMultiplier);
		break;
	case DamageType::Light:
		damage = static_cast<int>(damage_ * m_attributes.lightMultiplier);
		break;
	default:
		break;
	}

	if (m_isDead || damage <= 0) return;

	if (m_damageNumbers) {
		const sf::Vector2f& pos = getPosition();
		const sf::Vector2f& size = getSize();
		if (m_isInvincible) {
			m_damageNumbers->emitString(g_textProvider->getText("Immune"), sf::Vector2f(pos.x + 0.5f * size.x, pos.y), DamageNumberType::DamageOverTime);
			return;
		}
		if (overTime) {
			m_currentDotDamage += damage;
		} else {
			m_damageNumbers->emitNumber(damage, sf::Vector2f(pos.x + 0.5f * size.x, pos.y), DamageNumberType::Damage, critical);
		}
	}

	m_attributes.currentHealthPoints = std::max(0, std::min(m_attributes.maxHealthPoints, m_attributes.currentHealthPoints - damage));
	if (m_attributes.currentHealthPoints == 0) {
		setDead();
	}
	setSpriteColor(COLOR_DAMAGED, sf::milliseconds(400));
}

void LevelMovableGameObject::addDamageOverTime(DamageOverTimeData& data) {
	if (m_isDead || data.damageType == DamageType::VOID) return;
	// set the time for the first tick down, the first tick should not
	// hurt the mob. 
	data.duration -= sf::milliseconds(1);
	m_dots.push_back(data);
}

void LevelMovableGameObject::addHeal(int heal, bool overTime, bool critical) {
	if (m_isDead || heal <= 0) return;

	if (m_damageNumbers) {
		const sf::Vector2f& pos = getPosition();
		const sf::Vector2f& size = getSize();
		m_damageNumbers->emitNumber(heal, sf::Vector2f(pos.x + 0.5f * size.x, pos.y), overTime ? DamageNumberType::HealOverTime : DamageNumberType::Heal, critical);
	}

	m_attributes.currentHealthPoints = std::max(0, std::min(m_attributes.maxHealthPoints, m_attributes.currentHealthPoints + heal));
	setSpriteColor(COLOR_HEALED, sf::milliseconds(200));
}

void LevelMovableGameObject::setFeared(const sf::Time& fearedTime) {
	if (m_isDead) return;
	m_fearedTime = std::max(fearedTime, m_fearedTime);
}

void LevelMovableGameObject::setStunned(const sf::Time& stunnedTime) {
	if (m_isDead) return;
	m_stunnedTime = std::max(stunnedTime, m_stunnedTime);
}

void LevelMovableGameObject::onHit(Spell* spell) {
	if (m_isDead) return;

	spell->execOnHit(this);
	if (spell->getDamageType() == DamageType::VOID) return;
	addDamage(spell->getDamage(), spell->getDamageType(), false, spell->isCritical());
	if (spell->getDamagePerSecond() > 0.f && spell->getDuration() > sf::Time::Zero) {
		DamageOverTimeData data;
		data.damage = spell->getDamagePerSecond();
		data.damageType = spell->getDamageType();
		data.duration = spell->getDuration();
		addDamageOverTime(data);
	}
}

void LevelMovableGameObject::setDead() {
	if (m_isImmortal || m_isDead) return;
	// dispose the spells that this mob is an owner of and that are attached to it
	// that's how magic works, I guess?
	clearSpells(false);
	for (auto sc : *m_registeredSpellCreators) {
		sc->notifyMobDeath(this);
	}
	m_attributes.currentHealthPoints = 0;
	m_isDead = true;
	g_resourceManager->playSound(getDeathSoundPath());
}

int LevelMovableGameObject::getActiveSpellCount() const {
	int count = 0;
	for (auto& go : *m_screen->getObjects(_Spell)) {
		if (Spell* spell = dynamic_cast<Spell*>(go)) {
			if (spell->getOwner() == this) {
				++count;
			}
		}
	}
	return count;
}

void LevelMovableGameObject::clearSpells(bool clearAll) {
	for (auto& go : *m_screen->getObjects(_Spell)) {
		if (Spell* spell = dynamic_cast<Spell*>(go)) {
			if (spell->getOwner() == this) {
				spell->onOwnerDisposed();
				if (clearAll)
					spell->setDisposed();
			}
		}
	}

	// also check the toAdd vector in the screen to avoid nullptrs.
	for (auto& go : m_screen->getToAddObjects()) {
		if (Spell* spell = dynamic_cast<Spell*>(go)) {
			if (spell->getOwner() == this) {
				spell->onOwnerDisposed();
				if (clearAll)
					spell->setDisposed();
			}
		}
	}
}

void LevelMovableGameObject::registerSpellCreator(SpellCreator* creator) const {
	m_registeredSpellCreators->insert(creator);
}

void LevelMovableGameObject::executeSpell(int spell, const sf::Vector2f& target) {
	m_spellManager->setCurrentSpell(spell);
	m_spellManager->executeCurrentSpell(target, true);
}

sf::Time LevelMovableGameObject::executeDefaultFightAnimation(bool isBlocking, int times) {
	return m_movingBehavior->executeDefaultFightAnimation(isBlocking, times);
}

void LevelMovableGameObject::executeFightAnimation(const sf::Time& fightAnimationTime, GameObjectState animation, bool isBlocking) {
	m_movingBehavior->executeFightAnimation(fightAnimationTime, animation, isBlocking);
}

const Level* LevelMovableGameObject::getLevel() const {
	return m_level;
}

bool LevelMovableGameObject::isFacingRight() const {
	if (m_movingBehavior == nullptr) return true;
	return m_movingBehavior->isFacingRight();
}

bool LevelMovableGameObject::isUpsideDown() const {
	return m_movingBehavior->isUpsideDown();
}

bool LevelMovableGameObject::isIgnoreDynamicTiles() const {
	return m_movingBehavior->isIgnoreDynamicTiles();
}

bool LevelMovableGameObject::isDead() const {
	return m_isDead;
}

bool LevelMovableGameObject::isImmortal() const {
	return m_isImmortal;
}

bool LevelMovableGameObject::isStunned() const {
	return m_stunnedTime > sf::Time::Zero;
}

bool LevelMovableGameObject::isFeared() const {
	return m_fearedTime > sf::Time::Zero;
}

bool LevelMovableGameObject::isIgnoringCollision() const {
	return m_isIgnoringCollision;
}

bool LevelMovableGameObject::isReady() const {
	return m_movingBehavior->isReady();
}

void LevelMovableGameObject::setReady() {
	m_movingBehavior->setReady();
}

void LevelMovableGameObject::clearDots() {
	m_dots.clear();
}

void LevelMovableGameObject::setInvincible(bool value) {
	m_isInvincible = value;
}

bool LevelMovableGameObject::collides(const sf::Vector2f& nextPos) const {
	WorldCollisionQueryRecord rec;
	rec.ignoreDynamicTiles = isIgnoreDynamicTiles();
	sf::FloatRect nextBoundingBox = *getBoundingBox();
	nextBoundingBox.left = nextPos.x;
	nextBoundingBox.top = nextPos.y;
	rec.boundingBox = nextBoundingBox;
	auto bb = getBoundingBox();
	rec.collisionDirection = bb->top < nextBoundingBox.top ? CollisionDirection::Down : CollisionDirection::Up;
	return m_level->collides(rec);
}

void LevelMovableGameObject::loadBehavior() {
	delete m_attackingBehavior;
	m_attackingBehavior = createAttackingBehavior();

	delete m_movingBehavior;
	m_movingBehavior = createMovingBehavior();
}

void LevelMovableGameObject::loadResources() {
	g_resourceManager->loadTexture(getSpritePath(), ResourceType::Level);
	g_resourceManager->loadSoundbuffer(getDeathSoundPath(), ResourceType::Level);
}

void LevelMovableGameObject::flipGravity() {
	m_movingBehavior->flipGravity();
	m_animatedSprite.setFlippedY(m_movingBehavior->isUpsideDown());
}

void LevelMovableGameObject::setFacingRight(bool value) {
	if (!m_movingBehavior) return;
	m_movingBehavior->setFacingRight(value);
}

void LevelMovableGameObject::setCollisionTiltSuppressed() {
	if (!m_movingBehavior) return;
	m_movingBehavior->setCollisionTiltSuppressed();
}

GameObjectState LevelMovableGameObject::getState() const {
	return m_state;
}

SpellManager* LevelMovableGameObject::getSpellManager() const {
	return m_spellManager;
}

MovingBehavior* LevelMovableGameObject::getMovingBehavior() const {
	return m_movingBehavior;
}

AttackingBehavior* LevelMovableGameObject::getAttackingBehavior() const {
	return m_attackingBehavior;
}

AnimatedSprite& LevelMovableGameObject::getAnimatedSprite() {
	return m_animatedSprite;
}

const AttributeData* LevelMovableGameObject::getAttributes() const {
	return &m_attributes;
}

bool LevelMovableGameObject::isEating() const {
	return m_foodAttributes.first > sf::Time::Zero;
}

void LevelMovableGameObject::consumeFood(const sf::Time& duration, const AttributeData& attributes) {
	if (duration <= sf::Time::Zero) return;

	if (isEating()) {
		// old food attributes have to be removed
		m_attributes.removeBean(m_foodAttributes.second);
	}
	m_foodAttributes = std::pair<sf::Time, AttributeData>(duration, attributes);
	m_attributes.addBean(attributes);
}

bool LevelMovableGameObject::isAlly() const {
	return false;
}

float LevelMovableGameObject::getConfiguredMaxVelocityX() const {
	return m_movingBehavior ? m_movingBehavior->getMaxVelocityX() : 0.f;
}

float LevelMovableGameObject::getConfiguredMaxVelocityYDown() const {
	return m_movingBehavior ? m_movingBehavior->getMaxVelocityYDown() : 0.f;
}

float LevelMovableGameObject::getConfiguredMaxVelocityYUp() const {
	return m_movingBehavior ? m_movingBehavior->getMaxVelocityYUp() : 0.f;
}