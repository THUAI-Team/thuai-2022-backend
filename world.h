#ifndef WORLD_H
#define WORLD_H

#include "cpp_record_sdk/entities.h"

namespace thuai {
  const double RADIUS, GOAL_LENGTH, GOAL_WIDTH;

  struct World {
    Player* players[PLAYER_COUNT];
    Egg* eggs[EGG_COUNT];

    World();
    ~World();
  };
}
#endif