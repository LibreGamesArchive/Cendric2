#include "GUI/Window.h"

using namespace std;

Window::Window(const sf::FloatRect& box, WindowOrnamentStyle style)
{
	// using default values for constructor.
	m_size = sf::Vector2f(box.width, box.height);
	setSpriteOffset(sf::Vector2f(0.f, 0.f));
	setBoundingBox(box);

	m_mainLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_rounded_rectangle), CENDRIC_COLOR_BLACK, box.width, box.height);

	m_backLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_rounded_rectangle), CENDRIC_COLOR_LIGHT_PURPLE, box.width, box.height);

	if (style == WindowOrnamentStyle::NONE) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_none), CENDRIC_COLOR_WHITE, box.width, box.height);
	}
	else if (style == WindowOrnamentStyle::SMALL) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_small), CENDRIC_COLOR_WHITE, box.width, box.height);
	}
	else if (style == WindowOrnamentStyle::MEDIUM) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_medium), CENDRIC_COLOR_WHITE, box.width, box.height);
	}
	else if (style == WindowOrnamentStyle::LARGE) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_large), CENDRIC_COLOR_WHITE, box.width, box.height);
	}

	setPosition(sf::Vector2f(box.left, box.top));
}

Window::Window(const sf::FloatRect& box, WindowOrnamentStyle style, const sf::Color& mainColor, const sf::Color& backColor, const sf::Color& ornamentColor)
{
	m_size = sf::Vector2f(box.width, box.height);
	setSpriteOffset(sf::Vector2f(0.f, 0.f));
	setBoundingBox(box);

	m_mainLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_rounded_rectangle), mainColor, box.width, box.height);

	m_backLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_rounded_rectangle), backColor, box.width, box.height);

	if (style == WindowOrnamentStyle::NONE) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_none), ornamentColor, box.width, box.height);
	}
	else if (style == WindowOrnamentStyle::SMALL) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_small), ornamentColor, box.width, box.height);
	}
	else if (style == WindowOrnamentStyle::MEDIUM) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_medium), ornamentColor, box.width, box.height);
	}
	else if (style == WindowOrnamentStyle::LARGE) {
		m_ornamentLayer = SlicedSprite(g_resourceManager->getTexture(ResourceID::Texture_GUI_ornament_large), ornamentColor, box.width, box.height);
	}

	setPosition(sf::Vector2f(box.left, box.top));
}

void Window::setPosition(const sf::Vector2f& position)
{
	GameObject::setPosition(position);
	m_mainLayer.setPosition(position);
	m_backLayer.setPosition(position);
	m_ornamentLayer.setPosition(position);
}

void Window::render(sf::RenderTarget& renderTarget)
{
	renderTarget.draw(m_backLayer);
	renderTarget.draw(m_mainLayer);
	renderTarget.draw(m_ornamentLayer);
}

sf::Vector2f Window::getSize() const
{
	return m_size;
}

GameObjectType Window::getConfiguredType() const
{
	return GameObjectType::_Window;
}