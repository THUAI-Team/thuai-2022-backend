#ifndef WORLD_H
#define WORLD_H

#include "nlohmann/json.hpp"
#include "cpp_record_sdk/entities.h"

namespace thuai {
  const double RADIUS = 40.0, GOAL_LENGTH = 10.0, GOAL_WIDTH = 5.0;

  struct World {
    Player* players[PLAYER_COUNT];
    Egg* eggs[EGG_COUNT];

    nlohmann::json output_to_ai(int) const;

    World();
    ~World();
  };
}
#endif