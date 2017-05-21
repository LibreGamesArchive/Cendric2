#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include "Structs/WorldData.h"
#include "Structs/NPCData.h"
#include "Structs/MapDynamicTileData.h"
#include "FloatTriangle.h"

struct MapData final : WorldData {
	std::vector<MapDynamicTileData> dynamicTiles;

	std::vector<NPCData> npcs;
	std::vector<sf::FloatRect> collidableRects;
	std::vector<FloatTriangle> collidableTriangles;

	bool explorable = false;
};