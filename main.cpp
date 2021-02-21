#include <fstream>
#include <iostream>
#include <functional>
#include <memory>

#include "nlohmann/json.hpp"
#include "cpp_record_sdk/entities.h"
#include "cpp_record_sdk/record.h"
#include "world.h"
#include "stream_helper.h"

using json = nlohmann::json;
using namespace thuai;

const int FPS = 60, FRAME_COUNT = FPS * 60 * 2, FRAMES_PER_STATE = 6;

int main(void) {
  json init_msg, config;
  read_from_judger(init_msg);
  if (init_msg["player_num"] != 3) {
    return 1; // need 3 players
  }

  Record record(FPS);
  std::shared_ptr<World> world(std::make_shared<World>());

  json init_config = R"({"state": 0, "time": 0.1, "length": 4096})"_json;

  write_to_judger(init_config, -1);

  for (int cur_frame = 0; cur_frame < FRAME_COUNT; ++cur_frame) {
    ;// <-- do the simulation here
    int state = cur_frame / FRAMES_PER_STATE + 1; // ensure that cur_state > 0
    if (cur_frame % FRAMES_PER_STATE == 0) {
      // handle the interaction every 0.1s
      // send game state first
      auto msg = world->output_to_ai();
      write_to_judger(json({
        {"state", state},
        {"listen", {0, 1, 2}},
        {"player", {0, 1, 2}},
        {"content", {msg, msg, msg}}
      }), -1);
      // wait for ai reply
      bool round_end = false, received_info[] = {false, false, false};
      while (!round_end) {
        json incoming_msg;
        read_from_judger(incoming_msg);
        if (incoming_msg["player"] >= 0) {
          auto detail = json::parse(incoming_msg["content"]);
          if (detail["state"] == state) { // ensure that state is synchronized
            received_info[incoming_msg["player"]] = true;
            // parse the action of AI: walking/stopped/running; which egg to try grabbing
            /*
            {
              "action": "stopped" | "walking" | "running"
              "facing": Vector2,
              "grab"?: number // try to grab? null for nothing
              "drop"?: number // try to drop? radian / null
            }
            */
            for (int i = 0; i < 4; i++) {
              auto player_action = detail["actions"][i];
              int player_id = i + incoming_msg["player"] * 4;
              auto current_player = world->players[player_id];
              if (current_player->status() == SLIPPED) {
                continue; // when slipped, a player cannot do anything
              }

              //--------- handle movement ---------
              if (player_action["action"] == "running" && world->players[player_id]->endurance() <= 0) {
                player_action["action"] = "walking"; // unify all cases of walking
              }
              
              Vec2D facing; 
              facing.x = player_action["facing"]["x"]; facing.y = player_action["facing"]["y"];
              facing = facing.normalized();
              if (player_action["action"] == "running") {
                current_player->set_status(RUNNING);
                current_player->set_velocity(Vec2D{facing.x * RUN_SPEED, facing.y * RUN_SPEED});
              } else if (player_action["action"] == "walking") {
                current_player->set_status(WALKING);
                int egg_holding = current_player->egg();
                double walk_speed = WALK_SPEED_EMPTY;
                if (egg_holding != -1) {
                  walk_speed = get_walk_speed_with_egg(world->eggs[egg_holding]->score());
                }
                current_player->set_velocity(Vec2D{facing.x * walk_speed, facing.y * walk_speed});
              } else if (player_action["action"] == "stopped") {
                current_player->set_status(STOPPED);
                current_player->set_velocity(Vec2D{.0, .0});
              }
              //--------- handle egg placement ---------
              if (! player_action["drop"].is_null()) {
                const double radian = player_action["drop"];

              }
              if (! player_action["grab"].is_null()) {
                const double radian = player_action["drop"];
                
              }
            }
          }
        } else {
          auto error_content = json::parse(incoming_msg["content"]);
          if (error_content["error"] == 0) {
            received_info[error_content["player"]] = true;
          } else if (error_content["error"] == 1) {
            assert(error_content["player"] == state);
            round_end = true;
          }
        }
        round_end |= received_info[0] && received_info[1] && received_info[2];
      }

      // do the handling together
    }
  }
  {
    int r_score = 111, y_score = 222, b_score = 333;// calculate score
    record.set_score(r_score, y_score, b_score);
    json end_info = {
      {"0", r_score},
      {"1", y_score},
      {"2", b_score}
    };
    write_to_judger(json({
      {"state", -1},
      {"end_info", end_info.dump()}
    }), -1);
  }

  std::ofstream of(init_msg["replay"], std::ios::binary);
  record.serialize(of);

  return 0;
}