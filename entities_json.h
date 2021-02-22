#ifndef ENTITIES_JSON_H
#define ENTITIES_JSON_H

#include "nlohmann/json.hpp"
#include "cpp_record_sdk/entities.h"

namespace thuai {
  using json = nlohmann::json;
  void to_json(json& j, const Vec2D& vec);
  void from_json(const json& j, Vec2D& vec);

  NLOHMANN_JSON_SERIALIZE_ENUM(PlayerStatus, {
    {STOPPED, "stopped"},
    {WALKING, "walking"},
    {RUNNING, "running"},
    {SLIPPED, "slipped"},
  })
}

#endif