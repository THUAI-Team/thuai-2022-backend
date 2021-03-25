#include <fstream>
#include <functional>
#include <iostream>
#include <memory>

#include "cpp_record_sdk/entities.h"
#include "cpp_record_sdk/record.h"
#include "entities_json.h"
#include "nlohmann/json.hpp"
#include "stream_helper.h"
#include "world.h"

using json = nlohmann::json;
using namespace thuai;

constexpr const int FPS = 60, FRAME_COUNT = FPS * 20, FRAMES_PER_STATE = 6; // FIXME: 120 seconds!
const int32 velocityIterations = 10, positionIterations = 8;
int main(void) {

  json init_msg, config;
  read_from_judger(init_msg);
  if (init_msg["player_num"] != 3) {
    return 1; // need 3 players
  }
  bool is_offline[] = {false, false, false};
  for (int i = 0; i < 3; i++) {
    if (init_msg["player_list"][i] == 0) {
      is_offline[i] = true;
    }
  }

  Record record(FPS);
  std::shared_ptr<World> world(std::make_shared<World>());

  auto init_config =
      R"({"state": 0, "time": 3, "length": 4096})"_json; // TODO: time=0.1

  write_to_judger(init_config, -1);

  for (int cur_frame = 0; cur_frame < FRAME_COUNT; ++cur_frame) {
    std::cerr << "Current frame = " << cur_frame << std::endl;
    record.add_frame();
    if (!world->Update(FPS, velocityIterations, positionIterations)) {
      std::cerr << "Something Went Wrong!" << std::endl; // err occurs
    }

    int state = cur_frame / FRAMES_PER_STATE + 1; // ensure that cur_state > 0
    if (cur_frame % FRAMES_PER_STATE == 0) {
      // handle the interaction every 0.1s
      // send game state first
      std::cerr << "Now sending game state" << std::endl;
      auto msg = world->output_to_ai(state);
      write_to_judger(
          json(
              {{"state", state},
               {"listen", {0, 1, 2}},
               {"player", {0, 1, 2}},
               {"content",
                {
                    msg.patch(
                           R"([{ "op": "add", "path": "/team", "value": 0 }])"_json)
                        .dump(),
                    msg.patch(
                           R"([{ "op": "add", "path": "/team", "value": 1 }])"_json)
                        .dump(),
                    msg.patch(
                           R"([{ "op": "add", "path": "/team", "value": 2 }])"_json)
                        .dump(),
                }}}),
          -1);
      // wait for ai reply
      std::cerr << "Waiting for reply" << std::endl;
      bool round_end = false, received_info[] = {false, false, false};
      while (!round_end) {
        json incoming_msg;
        read_from_judger(incoming_msg);
        if (incoming_msg["player"] >= 0) {
          auto detail = json::parse(std::string(incoming_msg["content"]));
          std::cerr << "GOT DETAIL:" << detail << std::endl;
          if (detail["state"] == state) { // ensure that state is synchronized
            received_info[incoming_msg["player"]] = true;
            // parse the action of AI: walking/stopped/running; which egg to try
            // grabbing
            world->read_from_team_action(incoming_msg["player"], detail);
          }
        } else {
          auto error_content =
              json::parse(std::string(incoming_msg["content"]));
          std::cerr << "ERROR:" << error_content << std::endl;
          if (error_content["error"] == 0) {
            is_offline[error_content["player"]] = true; // 掉线了
          } else if (error_content["error"] == 1) {
            assert(error_content["state"] == state);
            round_end = true;
          }
        }
        round_end |= (received_info[0] || is_offline[0]) &&
                     (received_info[1] || is_offline[1]) &&
                     (received_info[2] || is_offline[2]);
      }

      std::cerr << "Waiting for reply done;" << std::endl;

      std::cerr << "received_info:";
      for (int i = 0; i < 3; i++)
        std::cerr << received_info[i] << ' ';
      std::cerr << std::endl;

      std::cerr << "is_offline:";
      for (int i = 0; i < 3; i++)
        std::cerr << is_offline[i] << ' ';
      std::cerr << std::endl;
    }
    for (int i = 0; i < PLAYER_COUNT; i++) {
      record.set_player_in_frame(*(world->players[i]));
    }
    for (int i = 0; i < EGG_COUNT; i++) {
      record.set_egg_in_frame(*(world->eggs[i]));
    }
  }
  
  int r_score = world->score[0], y_score = world->score[1],
      b_score = world->score[2]; // calculate score
  record.set_score(r_score, y_score, b_score);
  json end_info = {{"0", r_score}, {"1", y_score}, {"2", b_score}};
  write_to_judger(json({{"state", -1}, {"end_info", end_info.dump()}}), -1);

  std::ofstream of(std::string(init_msg["replay"]), std::ios::binary);
  record.serialize(of);
  of.flush();
  of.close();

  return 0;
}