#ifndef WORLD_H
#define WORLD_H

#include <box2d/box2d.h>

#include "cpp_record_sdk/entities.h"
#include "nlohmann/json.hpp"

namespace thuai {
const inline double DIAMETER = 40.0, GOAL_LENGTH = 10.0, GOAL_WIDTH = 5.0, RUN_SPEED = 4.0, WALK_SPEED_EMPTY = 2.0;

double get_walk_speed_with_egg(double);
struct World {

  Player* players[PLAYER_COUNT];
  Egg* eggs[EGG_COUNT];

  b2Body* b2players[PLAYER_COUNT];
  b2Body* b2eggs[EGG_COUNT];
  b2World* b2world{nullptr};

  bool Update();
  nlohmann::json output_to_ai() const;
  World();
  ~World();
};
}  // namespace thuai
#endif