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
std::map<Player *, int> slip_players_list;

typedef struct b2bodydata {
  Player *m_p_player{nullptr};
  Egg *m_p_egg{nullptr};
  b2Body *m_p_b2body{nullptr};
  b2bodydata(Player *_player, b2Body *_b2body)
      : m_p_player(_player), m_p_b2body(_b2body) {}
  b2bodydata(Egg *_egg, b2Body *_b2body) : m_p_egg(_egg), m_p_b2body(_b2body) {}
} b2bodydata;
void World::ContactListener::BeginContact(b2Contact *contact) {
  // Before collision, modified if needed
  return;
}

void World::ContactListener::EndContact(b2Contact *contact) {
  // After collision
  std::vector<b2Body *> collision_items;
  collision_items.push_back(contact->GetFixtureA()->GetBody());
  collision_items.push_back(contact->GetFixtureB()->GetBody());

  for (auto item : collision_items) {
    auto body_data = static_cast<b2bodydata *>(item->GetUserData());
    if (body_data != nullptr && body_data->m_p_player != nullptr) {
      body_data->m_p_b2body->SetLinearVelocity({.0, .0});
      body_data->m_p_player->set_status(PlayerStatus::SLIPPED);
      slip_players_list[body_data->m_p_player] = SLIP_FRAMES;
    }
  }
}

double get_walk_speed_with_egg(double egg_score) {
  return 3 - pow(1.07, egg_score - 10);
}
double get_distance(const Vec2D &pos1, const Vec2D &pos2) {
  return sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) +
              (pos1.y - pos2.y) * (pos1.y - pos2.y));
}
double get_distance(const b2Body *const obj1, const b2Body *const obj2) {
  return sqrt((obj1->GetPosition().x - obj2->GetPosition().x) *
                  (obj1->GetPosition().x - obj2->GetPosition().x) +
              (obj1->GetPosition().y - obj2->GetPosition().y) *
                  (obj1->GetPosition().y - obj2->GetPosition().y));
}
double get_distance(const b2Body *const obj1, const Vec2D &pos2) {
  return sqrt(
      (obj1->GetPosition().x - pos2.x) * (obj1->GetPosition().x - pos2.x) +
      (obj1->GetPosition().y - pos2.y) * (obj1->GetPosition().y - pos2.y));
}
double get_distance(const Vec2D &pos1, const b2Body *const obj2) {
  return sqrt(
      (pos1.x - obj2->GetPosition().x) * (pos1.x - obj2->GetPosition().x) +
      (pos1.y - obj2->GetPosition().y) * (pos1.y - obj2->GetPosition().y));
}

World::World() {
  {
    const double angle_delta = 2 * pi / 15, player_radius = DIAMETER / 4;
    double angle = .0;
    for (int i = 0; i < PLAYER_COUNT; i++) {
      if (i % 4 == 0)
        angle += angle_delta;
      players[i] = new Player(i);
      players[i]->set_position(
          {player_radius * cos(angle), player_radius * sin(angle)});
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
            {egg_radius * cos(angle), egg_radius * sin(angle)});
      }
    }
  }
  // ========== Box2d Related ==========
  {
    // Initialize b2World
    b2world = new b2World(b2Vec2(0, 0)); // Set gravity to 0.0
    this->m_contactlistener = new ContactListener();
    b2world->SetContactListener(m_contactlistener);
    b2BodyDef groundBodyDef;
    int vertex_count = 0;
    b2Vec2 ve[720];
    for (int i = 0; i < 360;
         i++) { // use degree not rad to create bound here for smoother egde
      for (int k = 0; k < 3; k++) { // three goals
        if (i > 60 * k - ceil(asin(GOAL_LENGTH / DIAMETER) / pi * 180) &&
            i < 60 * k + ceil(asin(GOAL_LENGTH / DIAMETER) / pi * 180)) {
          // goal's inner edge vertex is at 45.522 74.478 (degree), so on the
          // circle vertex at 45 and 75 is needed , 46 and 74 is not the asin's
          // value is 14.4775
          ve[vertex_count++].Set(
            (DIAMETER / 2) *
              cos((1 + 2 * k) * pi / 3 - asin(GOAL_LENGTH / DIAMETER)),
            (DIAMETER / 2) *
              sin((1 + 2 * k) * pi / 3 -
                  asin(GOAL_LENGTH /
                       DIAMETER))); // vertex on circle (goal's inner edge)

          ve[vertex_count++].Set(
            (DIAMETER / 2 + GOAL_WIDTH) *
              cos((1 + 2 * k) * pi / 3 - asin(GOAL_LENGTH / DIAMETER)),
            (DIAMETER / 2 + GOAL_WIDTH) *
              sin((1 + 2 * k) * pi / 3 -
                  asin(GOAL_LENGTH / DIAMETER))); // vertex on goal's outer edge
        }

        ve[vertex_count++].Set(
          (DIAMETER / 2) * cos(i / 180 * pi),
          (DIAMETER / 2) * sin(i / 180 * pi)); // vertex to create a circle
      }
    }
    groundBodyDef.position.Set(.0f, .0f);
    b2Body *groundBody = b2world->CreateBody(&groundBodyDef);

    b2ChainShape groundChain;
    groundChain.CreateLoop(ve, vertex_count);
    // finish the goal region

    groundBody->CreateFixture(&groundChain, .0f);
  }
  {
    // Initialize Players & eggs
    for (int i = 0; i < PLAYER_COUNT; i++) {
      b2BodyDef bodyDef;
      bodyDef.type = b2_dynamicBody;
      bodyDef.position.Set(static_cast<float>(players[i]->position().x),
                           static_cast<float>(players[i]->position().y));
      b2players[i] = b2world->CreateBody(&bodyDef);
      b2players[i]->SetUserData(new b2bodydata{players[i], b2players[i]});
      b2CircleShape dynamicBox;
      dynamicBox.m_radius = static_cast<float>(PLAYER_RADIUS);
      b2FixtureDef fixtureDef;
      fixtureDef.shape = &dynamicBox;
      fixtureDef.friction = 0;
      b2players[i]->CreateFixture(&fixtureDef);
      b2MassData massdata;
      massdata.mass = 50;
      massdata.center = b2Vec2(0, 0);
      massdata.I = 1.44;
      b2players[i]->SetMassData(&massdata);
      slip_players_list[players[i]] = -1;
    }

    for (int i = 0; i < EGG_COUNT; i++) {
      addEgg(i);
    }
  }
}

World::~World() {
  for (int i = 0; i < PLAYER_COUNT; i++)
    delete players[i];
  for (int i = 0; i < EGG_COUNT; i++)
    delete eggs[i];
  delete b2world;
}

void World::read_from_team_action(Team team, nlohmann::json detail) {
  std::map<int, std::pair<int, double>>
      grab_list; // {eggindex: (currentNearestPlayer,currentNearestDistance)}
                // only eggs on the ground
  std::map<int, std::pair<int, double>>
      grab_from_player_list; // {**PLAYERINDEX**:
                          // (currentNearestPlayer,currentNearestDistance)}
                          // only eggs on other players
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
    current_player->set_facing(facing);
    auto new_status = player_action["action"].get<PlayerStatus>();
    if (new_status != SLIPPED) {
      current_player->set_status(new_status);
    }
    //--------- handle egg placement ---------
    if (!player_action["drop"].is_null()) {
      const double radian = player_action["drop"];
      // resolve if two eggs to be placed at conflicted posistion
      double mindis = EGG_RADIUS + std::min(EGG_RADIUS, PLAYER_RADIUS);
      Vec2D pos_to_be_placed = {b2players[player_id]->GetPosition().x +
                                    cos(radian) * (PLAYER_RADIUS + EGG_RADIUS),
                                b2players[player_id]->GetPosition().y +
                                    sin(radian) * (PLAYER_RADIUS + EGG_RADIUS)};
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
        if (fabs(player->GetPosition().x - this->players[player_id]->position().x) < 1e-5
         && fabs(player->GetPosition().y - this->players[player_id]->position().y) < 1e-5) {
            continue;
        }

        if (get_distance(player, pos_to_be_placed) <= EGG_RADIUS + PLAYER_RADIUS) {
          can_be_placed = false;
          break;
        }
      }
      if (can_be_placed) {
        eggs[players[player_id]->egg()]->set_position(pos_to_be_placed);
        eggs[players[player_id]->egg()]->set_velocity({.0, .0});
        addEgg(players[player_id]->egg());
        players[player_id]->set_egg(-1);
      }
    }
    if (!player_action["grab"].is_null()) {
      const int egg_target = static_cast<int>(player_action["grab"]);

      if (egg_target > 0 && egg_target < EGG_COUNT &&
          egg_target != players[player_id]->egg()) {
        //  make sure only the nearest player grab the egg
        Vec2D pos_of_player = {b2players[player_id]->GetPosition().x,
                               b2players[player_id]->GetPosition().y};

        // if egg_target is not in the ground and therefore on some player
        if (b2eggs[egg_target] == nullptr) {
          for (int i = 0; i < PLAYER_COUNT; i++) {
            if (players[i]->egg() == egg_target) {
              auto dis = get_distance(b2eggs[egg_target], b2players[i]);
              if (dis <= EGG_RADIUS + MIN_GRAB_DIS) {
                if (grab_from_player_list.find(i) == grab_from_player_list.end() 
                  || grab_from_player_list[i].second > dis) {
                    grab_from_player_list[i] = std::make_pair(player_id, dis);
                }
              }
              break;
            }
          }
        } else {
          // the egg is on the ground
          auto dis = get_distance(b2eggs[egg_target], pos_of_player);
          if (dis <= EGG_RADIUS + MIN_GRAB_DIS) {
            if (grab_list.find(egg_target) == grab_list.end() || grab_list[egg_target].second > dis) {
              grab_list[egg_target] = std::make_pair(player_id, dis);
            }
          }
        }
      }
    }
  }
  // Remove the egg in the world and set the player's egg data
  for (auto item : grab_list) {
    if (b2eggs[item.first] == nullptr)
      continue;
    players[item.second.first]->set_egg(item.first);
    b2world->DestroyBody(b2eggs[item.first]);
    b2eggs[item.first] = nullptr;
  }
  // Grab egg from other player
  for (auto item : grab_from_player_list) {
    players[item.second.first]->set_egg(players[item.first]->egg());
    players[item.first]->set_egg(-1);
  }
}

void thuai::World::addEgg(int index) {
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(static_cast<float>(eggs[index]->position().x),
                       static_cast<float>(eggs[index]->position().y));
  b2eggs[index] = b2world->CreateBody(&bodyDef);
  b2eggs[index]->SetUserData(new b2bodydata(eggs[index], b2eggs[index]));
  b2CircleShape dynamicBox;
  dynamicBox.m_radius = static_cast<float>(EGG_RADIUS);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.friction = 0;
  b2eggs[index]->CreateFixture(&fixtureDef);
  b2MassData massdata;
  massdata.mass = 30;
  massdata.center = b2Vec2(0, 0);
  massdata.I = 1.8375;
  b2eggs[index]->SetMassData(&massdata);
}

bool thuai::World::Update(int FPS, int32 velocityIterations,
                          int32 positionIterations) {
  try {
    static const float timestep = 1.0f / static_cast<float>(FPS);

    for (int i = 0; i < PLAYER_COUNT; i++) {
      auto currentPlayer = players[i];
      auto b2currentPlayer = b2players[i];
      bool isSpeedDown = false;
      
      // reduce player's velocity if distance satisfies part I
      double player_distance_to_origin = get_distance(b2currentPlayer, {0, 0});
      if (player_distance_to_origin <= OUTER_SPEED_REDUCE_RADIUS &&
          player_distance_to_origin >= INNER_SPEED_REDUCE_RADIUS)
        isSpeedDown = true;
      if (currentPlayer->status() == PlayerStatus::RUNNING) {
        if (currentPlayer->endurance() > 4.0f / static_cast<float>(FPS)) {
          currentPlayer->set_endurance(currentPlayer->endurance() -
                                       4.0f / static_cast<float>(FPS));
          b2currentPlayer->SetLinearVelocity(
              {float(currentPlayer->facing().x * RUN_SPEED),
               float(currentPlayer->facing().y * RUN_SPEED)});

        } else {
          currentPlayer->set_status(PlayerStatus::WALKING);
        }
      }
      if (currentPlayer->status() == PlayerStatus::WALKING) {
        float newendurance =
            currentPlayer->endurance() + 0.5f / static_cast<float>(FPS);
        currentPlayer->set_endurance(newendurance > 5 ? 5 : newendurance);
        int egg_holding = currentPlayer->egg();
        double walk_speed = WALK_SPEED_EMPTY;
        if (egg_holding != -1) {
          walk_speed = get_walk_speed_with_egg(eggs[egg_holding]->score());
        }
        b2currentPlayer->SetLinearVelocity(
            {float(currentPlayer->facing().x * walk_speed),
             float(currentPlayer->facing().y * walk_speed)});
      }
      if (currentPlayer->status() == PlayerStatus::STOPPED) {
        float newendurance =
            currentPlayer->endurance() + 1 / static_cast<float>(FPS);
        currentPlayer->set_endurance(newendurance > 5 ? 5 : newendurance);
        b2currentPlayer->SetLinearVelocity({.0, .0});
      }
      // reduce player's velocity if distance satisfies part II
      if (isSpeedDown) {
        b2currentPlayer->SetLinearVelocity(
            {static_cast<float>(std::min(
                 currentPlayer->facing().x * SPEED_ON_SPEED_REDUCE,
                 static_cast<double>(b2currentPlayer->GetLinearVelocity().x))),
             static_cast<float>(
                 std::min(currentPlayer->facing().y * SPEED_ON_SPEED_REDUCE,
                          static_cast<double>(
                              b2currentPlayer->GetLinearVelocity().y)))});
      }

    } // Player update ends

    for (int i = 0; i < EGG_COUNT; i++) {
      continue;
    } // Egg update ends

    // Process slipping players
    for (auto i = slip_players_list.begin(); i != slip_players_list.end();
         i++) {
      if (i->second == -1)
        continue;
      if (--(i->second) <= 0) {
        i->second = -1;
        i->first->set_status(PlayerStatus::STOPPED);
      }
    }

    b2world->Step(timestep, velocityIterations,
                  positionIterations); // do the simulation

    for (int i = 0; i < PLAYER_COUNT; i++) {
      players[i]->set_position(
          {b2players[i]->GetPosition().x, b2players[i]->GetPosition().y});
      players[i]->set_velocity({b2players[i]->GetLinearVelocity().x,
                                b2players[i]->GetLinearVelocity().y});
      players[i]->set_facing(players[i]->velocity().normalized());
      int holding = players[i]->egg();
      if (holding != -1) {
        eggs[holding]->set_position(players[i]->position());
        eggs[holding]->set_velocity(players[i]->velocity());
      }
    }

    for (int k = 0; k < 3; k++) {
        score[k] = 0;
    }

    for (int i = 0; i < EGG_COUNT; i++) {
      if (b2eggs[i] == nullptr)
        continue;
      eggs[i]->set_position(
          {b2eggs[i]->GetPosition().x, b2eggs[i]->GetPosition().y});
      eggs[i]->set_velocity(
          {b2eggs[i]->GetLinearVelocity().x, b2eggs[i]->GetLinearVelocity().y});
      if (get_distance(b2eggs[i], {0, 0}) > DIAMETER / 2) {
        double eggangle = atan2(static_cast<float>(b2eggs[i]->GetPosition().y),
                                static_cast<float>(b2eggs[i]->GetPosition().x));
        if (eggangle < 0) {
            eggangle += 2 * pi;
        }
        for (int k = 0; k < 3; k++) {
          if (eggangle < 2 * (k+1) * pi / 3) {
            score[k] += eggs[i]->score();
            break;
          }
        }
      }
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}

nlohmann::json World::output_to_ai(int state) const {
  using json = nlohmann::json;
  json ret;
  ret["state"] = state;
  ret["eggs"] = json::array();
  ret["teams"] = {json::array(), json::array(), json::array()};
  for (int i = 0; i < EGG_COUNT; i++) {
    ret["eggs"][i] = {
        {"position", eggs[i]->position()},
        {"holder", -1},
        {"score", eggs[i]->score()} // set this later
    };
  }
  for (int i = 0; i < PLAYER_COUNT; i++) {
    int team = players[i]->team(), sub_id = i % 4;
    auto facing = players[i]->velocity().normalized();
    int holding = players[i]->egg();
    if (holding != -1) {
      ret["eggs"][holding]["holder"] = i;
    }
    ret["teams"][team][sub_id] = {{"position", players[i]->position()},
                                  {"status", players[i]->status()},
                                  {"facing", facing}};
  }
  return ret;
}
} // namespace thuai
