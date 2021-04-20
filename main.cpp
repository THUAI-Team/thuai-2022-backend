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

constexpr const int FPS = 60, FRAME_COUNT = FPS * 120, FRAMES_PER_STATE = 6;

const int32 velocityIterations = 10, positionIterations = 8;
int main(int argc, char **argv) {
  const bool verbose = std::string(argv[0]).find("verbose") != std::string::npos;
  std::cerr << "Judger logic starting...\n";
  std::cerr << "Verbose mode is " << (verbose ? "on" : "off") << std::endl;
#ifdef LOCAL
  std::cerr << "Local debugging mode\n";
#else
  std::cerr << "Online judging mode\n";
#endif
  json init_msg, config;
  read_from_judger(init_msg);
  if (init_msg["player_num"] != 3) {
    std::cerr << "No enough player! Exiting...\n";
    return 1; // need 3 players
  }
  bool is_offline[] = {false, false, false};
  for (int i = 0; i < 3; i++) {
    if (init_msg["player_list"][i] == 0) {
      is_offline[i] = true;
    }
  }

  if (is_offline[0] && is_offline[1] && is_offline[2]) {
    std::cerr << "All participants offline! Exiting...\n";
    return 1;
  }

  Record record(FPS);
  std::shared_ptr<World> world(std::make_shared<World>());

  for (int i = 0; i < EGG_COUNT; i++) {
    record.init_egg_score(*(world->eggs[i]));
  }

#ifdef LOCAL
  // for debugging, time=10s, however on real platforms, time=0.1s
  auto init_config =
      R"({"state": 0, "time": 0.1, "length": 4096})"_json; // TODO: TIME LIMIT
#else
  auto init_config =
      R"({"state": 0, "time": 0.1, "length": 4096})"_json; 
#endif

  write_to_judger(init_config, -1);
  
  std::cerr << "Judger logic started.\n";

  for (int cur_frame = 0; cur_frame < FRAME_COUNT; ++cur_frame) {
    record.add_frame();


    int state = cur_frame / FRAMES_PER_STATE * 2 + 1; // ensure that cur_state > 0
    int aux_state = cur_frame / FRAMES_PER_STATE * 2 + 2; // monotonically increases

    if (!world->Update(FPS, velocityIterations, positionIterations)) {
      std::cerr << "Something Went Wrong! Logic Crashed."
                << std::endl;  // err occurs
    }
    if (verbose) {
      std::cerr << "Current frame = " << cur_frame << std::endl;
    }
 
    if (cur_frame % FRAMES_PER_STATE == 0) {
      // handle the interaction every 0.1s
      // send game state first

      if (verbose) {
        std::cerr << "Now sending game state" << std::endl;
      }
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
      // std::cerr << "Waiting for reply" << std::endl;
      bool round_end = is_offline[0] && is_offline[1] && is_offline[2],
        received_info[] = {false, false, false};
      while (!round_end) {
        json incoming_msg;
        if (verbose) {
          std::cerr << "Waiting for next message...\n"; 
        }
        read_from_judger(incoming_msg);
        if (verbose) {
          std::cerr << "Got incoming message: ###" << incoming_msg << "###" << std::endl;
        }
        if (incoming_msg["player"] >= 0) {
          auto detail = json::parse(std::string(incoming_msg["content"]));

          if (verbose) {
            std::cerr << "Got detail from player" << incoming_msg["player"] << ":" << detail << std::endl;
          } 
          if (detail["state"] == state && !received_info[(int)incoming_msg["player"]]) { 
            // ensure that state is synchronized, and this reply comes before "timeout" directive
            received_info[(int)incoming_msg["player"]] = true;
            // parse the action of AI: walking/stopped/running; which egg to try
            // grabbing
            world->read_from_team_action(incoming_msg["player"], detail);
          }
        } else {
          auto error_content =
              json::parse(std::string(incoming_msg["content"]));
          std::cerr << "Error from judger:" << error_content << std::endl;
          if (error_content["error"] == 0) {
            is_offline[(int)error_content["player"]] = true; // 掉线了
          } else if (error_content["error"] == 1) { // err in document: this should be an timeout
            received_info[(int)error_content["player"]] = true; // 超时了
          }
        }
        round_end |= (received_info[0] || is_offline[0]) &&
                     (received_info[1] || is_offline[1]) &&
                     (received_info[2] || is_offline[2]);
        if (verbose) {
          std::cerr << "received_info:";
          for (int i = 0; i < 3; i++) std::cerr << received_info[i] << " \n"[i == 2];
          std::cerr << "is_offline:";
          for (int i = 0; i < 3; i++) std::cerr << is_offline[i] << " \n"[i == 2];
          std::cerr << "round_end = " << round_end << std::endl;
        }
      }
      if (verbose) {
        std::cerr << "auxiliary:" << aux_state << std::endl;
      }
      write_to_judger(json({{"state", aux_state},
                            {"listen", json::array()},
                            {"player", json::array()},
                            {"content", json::array()}}),
                      -1);  // into assist state
    }
    for (int i = 0; i < PLAYER_COUNT; i++) {
      record.set_player_in_frame(*(world->players[i]));
    }
    for (int i = 0; i < EGG_COUNT; i++) {
      record.set_egg_in_frame(*(world->eggs[i]));
    }
  }

  std::cerr << "*****Game Ended!*****\n";

  int r_score = world->score[0], y_score = world->score[1],
      b_score = world->score[2]; // calculate score
  record.set_score(r_score, y_score, b_score);
  std::cerr << "Scores:" << r_score << ',' << y_score << ',' << b_score << '\n';

  std::cerr << "Writing replay to file...\n";
  std::ofstream of(std::string(init_msg.at("replay")), std::ios::binary);
  record.serialize(of);
  of.close();

  std::cerr << "Writing end_info..\n";
  json end_info = {{"0", r_score}, {"1", y_score}, {"2", b_score}};
  write_to_judger(json({{"state", -1}, {"end_info", end_info.dump()}}), -1);

  return 0;
}
