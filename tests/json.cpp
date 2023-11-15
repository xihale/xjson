#include "../lib/parser.hpp"
#include <format>
#include <stdexcept>
#include <string>
#include <source_location>
#include <iostream>
#include <assert.h>
#include <string_view>

using namespace xihale::json;

template<typename T, typename U>
void assert_equal(const T &a, const U &b, const std::source_location loc=std::source_location::current()){
  if(a!=b){
    std::cerr<<std::format("Line {} Column {}: {} != {}\n", loc.line(), loc.column(), a, b);
  }
}

int main(){

  assert_equal(std::string("hello"), std::string(json("\"hello\"")));
  assert_equal(true, bool(json("true")));
  assert_equal(1.23, double(json("1.23")));
  assert_equal(123, int(json("123")));
  assert(json("null").is_null());

  json j(R"({"a": 123})");

  assert(j.is_object());
  assert_equal(j["a"].operator int(), 123);

  json j2(R"({"a": [1, {"b": 2}, 3]})");

  assert(j2.is_object());
  assert(j2["a"].is_array());
  assert_equal(j2["a"][1]["b"].operator int(), 2);

  json j3(R"({
"a":{
"b":[
{"c": [2]}
]}})");

  assert(j3.is_object());
  assert(j3["a"].is_object());
  assert(j3["a"]["b"].is_array());
  assert(j3["a"]["b"][0ul]["c"].is_array());
  assert_equal(j3["a"]["b"][0ul]["c"][0ul].operator int(), 2);

  // assert_equal(json(R"({"a": "b"})"), json(R"({"a": "b"})"));

  return 0;
}