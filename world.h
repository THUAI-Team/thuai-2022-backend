#ifndef WORLD_H
#define WORLD_H

#include <box2d/box2d.h>

#include "cpp_record_sdk/entities.h"
#include "nlohmann/json.hpp"

namespace thuai {
using json = nlohmann::json;
const double DIAMETER = 40.0, GOAL_LENGTH = 10.0, GOAL_WIDTH = 5.0,
                    RUN_SPEED = 4.0, WALK_SPEED_EMPTY = 2.0;
const double PLAYER_RADIUS = .24, EGG_RADIUS = .35;
const double MIN_GRAB_DIS = .1 + PLAYER_RADIUS;
const double INNER_SPEED_REDUCE_RADIUS = 18.0, OUTER_SPEED_REDUCE_RADIUS = 20.0;

double get_walk_speed_with_egg(double);
struct World {

  Player *players[PLAYER_COUNT];
  Egg *eggs[EGG_COUNT];

  b2Body *b2players[PLAYER_COUNT];
  b2Body *b2eggs[EGG_COUNT];
  b2World *b2world{nullptr};
  void addEgg(int index);
  bool Update(int FPS, int32 velocityIterations,
              int32 positionIterations);
  json output_to_ai(int) const;
  void read_from_team_action(Team team, json detail);
  World();
  ~World();
};
} // namespace thuai
#endif