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
const double INNER_SPEED_REDUCE_RADIUS = 18.0,
             OUTER_SPEED_REDUCE_RADIUS = 20.0;
const double SPEED_ON_SPEED_REDUCE = .5;
const int SLIP_FRAMES = 90;
double
get_walk_speed_with_egg(double);
struct World{
  class ContactListener : public b2ContactListener
  {
    void BeginContact(b2Contact*);
    void EndContact(b2Contact*);
  };
  const double pi = acos(-1);
  int score[3];
  Player* players[PLAYER_COUNT];
  Egg* eggs[EGG_COUNT];
  int vertex_count = 0;
  b2Vec2 ve[360];
  b2Body* b2players[PLAYER_COUNT];
  b2Body* b2eggs[EGG_COUNT];
  b2World* b2world{ nullptr };
  void addEgg(int index);
  bool Update(int FPS, int32 velocityIterations, int32 positionIterations);
  json output_to_ai(int) const;
  void read_from_team_action(Team team, json detail);
  ContactListener* m_contactlistener{ nullptr };
  int pnpoly(Vec2D pos); // return if pos is within the playground
  World();
  ~World();
};

} // namespace thuai
#endif