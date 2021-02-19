#include <random>
#include <cmath>
#include "world.h"

const double thuai::RADIUS = 40.0, thuai::GOAL_LENGTH = 10.0, thuai::GOAL_WIDTH = 5.0;

thuai::World::World() {
  const double pi = acos(-1);
  {
    const double angle_delta = 2 * pi / (3 * 5), player_radius = RADIUS / 2;
    double angle = .0;
    for (int i = 0; i < PLAYER_COUNT; i++) {
      if (i % 4 == 0) angle += angle_delta;
      players[i] = new Player(i);
      players[i]->set_position({player_radius * cos(angle), player_radius * sin(angle)});
      angle += angle_delta;
    }
  }
  {
    std::random_device rd;
    std::mt19937 mtgen(rd());
    int egg_id = 0;
    for (double angle = 0; angle < 1.95 * pi; angle += 2 * pi / 3) {
      const double egg_radius_delta = RADIUS / 6;
      for (int k = 1; k <= 5; k++, egg_id++) {
        double egg_radius = egg_radius_delta * k;
        double score = mtgen() * 10.0 / std::mt19937::max() + 10.0;       // score: [10, 20)
        eggs[egg_id] = new Egg(egg_id, score);
        eggs[egg_id]->set_position({egg_radius * cos(angle), egg_radius * sin(angle)});
      }
    }
  }
}

thuai::World::~World() {
  for (int i = 0; i < PLAYER_COUNT; i++)
    delete players[i];
  for (int i = 0; i < EGG_COUNT; i++)
    delete eggs[i];
}