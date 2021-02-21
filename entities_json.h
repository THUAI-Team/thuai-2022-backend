#ifndef VEC2D_JSON_H
#define VEC2D_JSON_H

#include "nlohmann/json.hpp"
#include "cpp_record_sdk/entities.h"

namespace thuai {
  using json = nlohmann::json;
  void to_json(json& j, const Vec2D& vec) {
    j = json{
      {"x", vec.x},
      {"y", vec.y}
    };
  }
  void from_json(const json& j, Vec2D& vec) {
    j.at("x").get_to(vec.x);
    j.at("y").get_to(vec.y);
  }

  NLOHMANN_JSON_SERIALIZE_ENUM(PlayerStatus, {
    {STOPPED, "stopped"},
    {WALKING, "walking"},
    {RUNNING, "running"},
    {SLIPPED, "slipped"},
  })
}

#endif