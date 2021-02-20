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
            // parse the action of AI: walking/stopping/running; which egg to try grabbing
            // 

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