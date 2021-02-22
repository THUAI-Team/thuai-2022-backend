#include "entities_json.h"

namespace thuai {
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
}