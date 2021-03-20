#include "world.h"
#include "entities_json.h"
#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace thuai {
double
get_walk_speed_with_egg(double egg_score)
{
  return 3 - pow(1.07, egg_score - 10);
}
double
get_distance(const Vec2D& pos1, const Vec2D& pos2)
{
  return sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) +
              (pos1.y - pos2.y) * (pos1.y - pos2.y));
}
double
get_distance(const b2Body* const obj1, const b2Body* const obj2)
{
  return sqrt((obj1->GetPosition().x - obj2->GetPosition().x) *
                (obj1->GetPosition().x - obj2->GetPosition().x) +
              (obj1->GetPosition().y - obj2->GetPosition().y) *
                (obj1->GetPosition().y - obj2->GetPosition().y));
}
double
get_distance(const b2Body* const obj1, const Vec2D& pos2)
{
  return sqrt(
    (obj1->GetPosition().x - pos2.x) * (obj1->GetPosition().x - pos2.x) +
    (obj1->GetPosition().y - pos2.y) * (obj1->GetPosition().y - pos2.y));
}
double
get_distance(const Vec2D& pos1, const b2Body* const obj2)
{
  return sqrt(
    (pos1.x - obj2->GetPosition().x) * (pos1.x - obj2->GetPosition().x) +
    (pos1.y - obj2->GetPosition().y) * (pos1.y - obj2->GetPosition().y));
}

World::World()
{
  const double pi = acos(-1);
  {
    const double angle_delta = 2 * pi / 15, player_radius = DIAMETER / 4;
    double angle = .0;
    for (int i = 0; i < PLAYER_COUNT; i++) {
      if (i % 4 == 0)
        angle += angle_delta;
      players[i] = new Player(i);
      players[i]->set_position(
        { player_radius * cos(angle), player_radius * sin(angle) });
      angle += angle_delta;
    }
  }
  {
    std::random_device rd;
    std::mt19937 mtgen(rd());
    int egg_id = 0;
    for (double angle = 0; angle < 1.95 * pi; angle += 2 * pi / 3) {
      const double egg_radius_delta = DIAMETER / 12;
      for (int k = 1; k <= 5; k++, egg_id++) {
        double egg_radius = egg_radius_delta * k;
        int score =
          int(mtgen() * 10.0 / std::mt19937::max() + 10.0); // score: [10, 20)
        eggs[egg_id] = new Egg(egg_id, score);
        eggs[egg_id]->set_position(
          { egg_radius * cos(angle), egg_radius * sin(angle) });
      }
    }
  }
  // ========== Box2d Related ==========
  {
    // Initialize b2World
    b2world = new b2World(b2Vec2(0, 0));
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(.0f, .0f);
    b2Body* groundBody = b2world->CreateBody(&groundBodyDef);

    b2CircleShape groundCircle;
    groundCircle.m_radius = (DIAMETER / 2);
    b2PolygonShape goalBox[3];
    for (auto& item : goalBox)
      item.SetAsBox(GOAL_LENGTH, GOAL_WIDTH);
    // TODO: finish the goal region

    groundBody->CreateFixture(&groundCircle, .0f);
  }
  {
    // Initialize Players & eggs
    for (int i = 0; i < PLAYER_COUNT; i++) {
      b2BodyDef bodyDef;
      bodyDef.type = b2_dynamicBody;
      bodyDef.position.Set(float(players[i]->position().x),
                           float(players[i]->position().y));
      b2players[i] = b2world->CreateBody(&bodyDef);
      b2CircleShape dynamicBox;
      dynamicBox.m_radius = PLAYER_RADIUS;
      b2FixtureDef fixtureDef;
      fixtureDef.shape = &dynamicBox;
      fixtureDef.friction = 0;
      b2players[i]->CreateFixture(&fixtureDef);
      b2MassData massdata;
      massdata.mass = 50;
      b2players[i]->SetMassData(&massdata);
    }

    for (int i = 0; i < EGG_COUNT; i++) {
    }
  }
}

World::~World()
{
  for (int i = 0; i < PLAYER_COUNT; i++)
    delete players[i];
  for (int i = 0; i < EGG_COUNT; i++)
    delete eggs[i];
  delete b2world;
}

void
World::read_from_team_action(Team team, nlohmann::json detail)
{
  std::map<int, std::pair<int, double>>
    grablist; // {eggindex: (currentNearestPlayer,currentNearestDistance)}
  for (int i = 0; i < 4; i++) {
    auto player_action = detail["actions"][i];
    int player_id = i + int(team) * 4;
    auto current_player = players[player_id];

    if (current_player->status() == SLIPPED) {
      continue; // when slipped, a player cannot do anything
    }

    //--------- handle movement ---------
    if (player_action["action"] == "running" &&
        (players[player_id]->endurance() <= 0 ||
         players[player_id]->egg() != -1)) {
      player_action["action"] = "walking"; // unify all cases of walking
    }

    Vec2D facing;
    player_action["facing"].get_to(facing);
    facing = facing.normalized();
    auto new_status = player_action["action"].get<PlayerStatus>();
    if (new_status != SLIPPED) {
      current_player->set_status(new_status);
    }
    //--------- handle egg placement ---------
    if (!player_action["drop"].is_null()) {
      const double radian = player_action["drop"];
      // resolve if two eggs to be placed at conflicted posistion
      double mindis = EGG_RADIUS + std::min(EGG_RADIUS, PLAYER_RADIUS);
      Vec2D pos_to_be_placed = { b2players[player_id]->GetPosition().x +
                                   sin(radian) * (PLAYER_RADIUS + EGG_RADIUS),
                                 b2players[player_id]->GetPosition().y +
                                   cos(radian) * (PLAYER_RADIUS + EGG_RADIUS) };
      bool can_be_placed = true;
      for (auto egg : b2eggs) {
        if (egg == nullptr)
          continue;
        if (get_distance(egg, pos_to_be_placed) <= EGG_RADIUS + EGG_RADIUS) {
          can_be_placed = false;
          break;
        }
      }
      for (auto player : b2players) {
        if (get_distance(player, pos_to_be_placed) <=
            EGG_RADIUS + PLAYER_RADIUS) {
          can_be_placed = false;
          break;
        }
      }
      if (can_be_placed) {
        addEgg(players[player_id]->egg());
        players[player_id]->set_egg(-1);
      }
    }
    if (!player_action["grab"].is_null()) {
      const double egg_target = player_action["grab"];
      //  make sure only the nearest player grab the egg
      Vec2D pos_to_be_grabbed = {
        b2players[player_id]->GetPosition().x + sin(egg_target) * MIN_GRAB_DIS,
        b2players[player_id]->GetPosition().y + cos(egg_target) * MIN_GRAB_DIS
      };

      for (int i = 0; i < EGG_COUNT; i++) {
        if (b2eggs[i] == nullptr)
          continue;
        if (auto dis =
              get_distance(b2eggs[i], pos_to_be_grabbed) <= EGG_RADIUS) {
          if (grablist.find(i) != grablist.end())
            if (grablist[i].second < dis)
              continue;
          grablist[i] = std::make_pair(player_id, dis);
          break;
        }
      }
    }
  }
  for (auto item : grablist) {
    if (b2eggs[item.first] == nullptr)
      continue;
    players[item.second.first]->set_egg(item.first);
    b2world->DestroyBody(b2eggs[item.first]);
    b2eggs[item.first] = nullptr;
  }
}

void
thuai::World::addEgg(int index)
{
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(float(eggs[index]->position().x),
                       float(eggs[index]->position().y));
  b2eggs[index] = b2world->CreateBody(&bodyDef);
  b2CircleShape dynamicBox;
  dynamicBox.m_radius = EGG_RADIUS;
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.friction = 0;
  b2eggs[index]->CreateFixture(&fixtureDef);
  b2MassData massdata;
  massdata.mass = 30;
  b2eggs[index]->SetMassData(&massdata);
}

bool
thuai::World::Update(int FPS,
                     int32 velocityIterations,
                     int32 positionIterations)
{
  try {
    static const float timestep = 1.0f / float(FPS);

    for (int i = 0; i < PLAYER_COUNT; i++) {
      auto currentPlayer = players[i];
      auto b2currentPlayer = b2players[i];
      bool isSpeedDown = false;
      // TODO: reduce player's velocity if distance satisfies
      if (currentPlayer->status() == PlayerStatus::RUNNING)
        if (currentPlayer->endurance() > 4.0f / float(FPS)) {
          currentPlayer->set_endurance(currentPlayer->endurance() -
                                       4.0f / float(FPS));
          b2currentPlayer->SetLinearVelocity(
            { float(currentPlayer->facing().x * RUN_SPEED),
              float(currentPlayer->facing().y * RUN_SPEED) });

        } else
          currentPlayer->set_status(PlayerStatus::WALKING);
      if (currentPlayer->status() == PlayerStatus::WALKING) {
        float newendurance = currentPlayer->endurance() + 0.5f / float(FPS);
        currentPlayer->set_endurance(newendurance > 5 ? 5 : newendurance);
        int egg_holding = currentPlayer->egg();
        double walk_speed = WALK_SPEED_EMPTY;
        if (egg_holding != -1) {
          walk_speed = get_walk_speed_with_egg(eggs[egg_holding]->score());
        }
        b2currentPlayer->SetLinearVelocity(
          { float(currentPlayer->facing().x * walk_speed),
            float(currentPlayer->facing().y * walk_speed) });
      }
      if (currentPlayer->status() == PlayerStatus::STOPPED) {
        float newendurance = currentPlayer->endurance() + 1 / float(FPS);
        currentPlayer->set_endurance(newendurance > 5 ? 5 : newendurance);
        b2currentPlayer->SetLinearVelocity({ .0, .0 });
      }

    } // Player update ends

    for (int i = 0; i < EGG_COUNT; i++) {
      continue; // TODO: eggs update here
    }           // Egg update ends

    b2world->Step(timestep,
                  velocityIterations,
                  positionIterations); // do the simulation

    for (int i = 0; i < PLAYER_COUNT; i++) {
      players[i]->set_position(
        { b2players[i]->GetPosition().x, b2players[i]->GetPosition().y });
      players[i]->set_velocity({ b2players[i]->GetLinearVelocity().x,
                                 b2players[i]->GetLinearVelocity().y });
    }

    for (int i = 0; i < EGG_COUNT; i++) {
      if (b2eggs[i] == nullptr)
        continue;
      eggs[i]->set_position(
        { b2eggs[i]->GetPosition().x, b2eggs[i]->GetPosition().y });
      eggs[i]->set_velocity(
        { b2eggs[i]->GetLinearVelocity().x, b2eggs[i]->GetLinearVelocity().y });
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}

nlohmann::json
World::output_to_ai(int state) const
{
  using json = nlohmann::json;
  json ret;
  ret["state"] = state;
  ret["eggs"] = json::array();
  ret["teams"] = { json::array(), json::array(), json::array() };
  for (int i = 0; i < EGG_COUNT; i++) {
    ret["eggs"][i] = {
      { "position", eggs[i]->position() }, { "holder", -1 } // set this later
    };
  }
  for (int i = 0; i < PLAYER_COUNT; i++) {
    int team = players[i]->team(), sub_id = i % 4;
    auto facing = players[i]->velocity().normalized();
    int holding = players[i]->egg();
    ret["eggs"][holding]["holder"] = i;
    ret["teams"][team][sub_id] = { { "position", players[i]->position() },
                                   { "status", players[i]->status() },
                                   { "facing", facing } };
  }
  return ret;
}
} // namespace thuai