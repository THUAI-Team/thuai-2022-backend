#include <iostream>
#include <string>
#include "include/json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

int main(void) {
	json j;
	std::cin >> j;
	std::cout << j["replay"];
	return 0;
}