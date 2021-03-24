#include <string>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include "stream_helper.h"

namespace thuai {
  void i32_to_bytes(int32_t val, bool big_endian, char *buf) {
    memcpy(buf, &val, 4ull);
    if (big_endian) {
      std::swap(buf[0], buf[3]);
      std::swap(buf[1], buf[2]);
    }
  }

  int32_t bytes_to_i32(bool big_endian, char* orig_buf) {
    char buf[4];
    memcpy(buf, orig_buf, 4);
    if (big_endian) {
      std::swap(buf[0], buf[3]);
      std::swap(buf[1], buf[2]);
    }
    return *reinterpret_cast<int32_t*>(buf);
  }

  void write_to_judger(json j, int target) {
    std::string s = j.dump();

    int32_t len = s.length();
    char buf[4];

    i32_to_bytes(len, true, buf); // 4: len    
    std::cout.write(buf, 4);

    i32_to_bytes(target, true, buf); // 4: target
    std::cout.write(buf, 4);

    std::cout << s; // content

    std::cout.flush();
  }

  void read_from_judger(json &j) {
    char len_buf[4], *json_buf;
    
    std::cin.read(len_buf, 4);
    auto len = bytes_to_i32(true, len_buf);

    json_buf = new char[len+1];
    json_buf[len] = '\0';
    std::cin.read(json_buf, len);
    j = json::parse(json_buf);
    delete[] json_buf;
  }
}