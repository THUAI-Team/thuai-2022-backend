#ifndef WORLD_H
#define WORLD_H

#include "Box2D/Box2D.h"
#include "cpp_record_sdk/entities.h"

namespace thuai {
const double RADIUS = 40.0, GOAL_LENGTH = 10.0, GOAL_WIDTH = 5.0;
const b2Vec2 gravity(.0, .0);

struct World {
  Player* players[PLAYER_COUNT];
  Egg* eggs[EGG_COUNT];

  World();
  ~World();
};
}  // namespace thuai
#endif