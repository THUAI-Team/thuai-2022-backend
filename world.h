#ifndef WORLD_H
#define WORLD_H

#include <box2d/box2d.h>

#include "cpp_record_sdk/entities.h"
#include "nlohmann/json.hpp"

namespace thuai {
const double RADIUS = 40.0, GOAL_LENGTH = 10.0, GOAL_WIDTH = 5.0;
struct World {
  Player* players[PLAYER_COUNT];
  Egg* eggs[EGG_COUNT];
  b2World* b2world{nullptr};
  nlohmann::json output_to_ai() const;

  World();
  ~World();
};
}  // namespace thuai
#endif