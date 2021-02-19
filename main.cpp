#include <fstream>
#include <iostream>
#include <memory>

#include "nlohmann/json.hpp"
#include "cpp_record_sdk/entities.h"
#include "cpp_record_sdk/record.h"
#include "world.h"

using json = nlohmann::json;
using namespace thuai;

const int FPS = 60, FRAME_COUNT = FPS * 60 * 2, FRAMES_PER_STATE = 6;

int main(void) {
  json init_msg, config;
  std::cin >> init_msg;
  if (init_msg["player_num"] != 3) {
    return 1; // need 3 players
  }

  Record record(FPS);
  std::shared_ptr<World> world(std::make_shared<World>());

  std::cout << R"({"state": 0, "time": 0.1, "length": 4096})";

  for (int cur_frame = 0; cur_frame < FRAME_COUNT; ++cur_frame) {
    ;// <-- do the simulation here
    int cur_state = cur_frame / FRAMES_PER_STATE + 1; // ensure that cur_state > 0
    if (cur_frame % FRAMES_PER_STATE == 0) {
      // handle the interaction every 0.1s
      // send game state first

    }
  }

  return 0;
}