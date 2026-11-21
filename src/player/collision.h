#ifndef MORPH_PLAYER_COLLISION_H
#define MORPH_PLAYER_COLLISION_H
#include "core/game_types.h"
/* Positive radius and rectangle dimensions; finite coordinates.
 * External tangency is NOT collision, matching the HTML's strict < test.
 * Polygon vertices follow the perimeter (either winding), including concave
 * simple polygons. Repeated vertices are accepted; fewer than 3 are invalid.
 * Invalid inputs return false. No allocation, state mutation or raylib needed. */
bool CollisionCircleRect(GameVec2 center, float radius, GameRect rect);
bool CollisionCirclePolygon(GameVec2 center, float radius, const GameVec2 *vertices, size_t count);
/* Swept circle against a stationary circle; strict external tangency. */
bool CollisionSweptCircle(GameVec2 from, GameVec2 to, float radius, GameVec2 center, float other_radius);
#endif
