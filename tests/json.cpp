#include <json.hpp>
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

int main()try{

  assert_equal(json(R"({"id":22645196,"name":"Bad Apple!!"})")["name"].to_string(), std::string("Bad Apple!!"));

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
{"c": ["Hello Json\nFor C++\n"]},
]}})");

  assert(j3.is_object());
  assert(j3["a"].is_object());
  assert(j3["a"]["b"].is_array());
  assert(j3["a"]["b"][0]["c"].is_array());
  assert_equal(j3["a"]["b"][0]["c"][0].operator std::string(), "Hello Json\nFor C++\n");
  
  json j4(R"({"a": {"b": [1,2,3]}})");
  assert_equal(j4.operator std::string(), R"({"a":{"b":[1,2,3]}})");

  json j5(R"({"a": "delete"})");

  (j5["a"]=json("{}")).insert("b", json("[1,2,3]"));
  assert_equal(j5["a"]["b"][1].operator int(), 2);

  std::string temp="aaa";

  json j6={
    {"a", 1},
    {"b", std::string("123")},
    {"c", {
      {"d", 3.3}
    }},
    {"temp", temp},
  };
  assert_equal(j6["c"]["d"].operator double(), 3.3);
  assert_equal(j6["b"].to_string(), "123");

  json j7(R"({"result":{"songs":[{"id":22645196,"name":"Bad Apple!!","artists":[{"id":17423,"name":"のみこ","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p1.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":2076221,"name":"Lovelight","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p1.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1179590400007,"size":11,"copyrightId":0,"status":1,"picId":109951166027157822,"mark":0},"duration":319426,"copyrightId":663018,"status":0,"alias":[],"rtype":0,"ftype":0,"mvid":290067,"fee":0,"rUrl":null,"mark":262144},{"id":33599494,"name":"Bad Apple","artists":[{"id":12342149,"name":"Lizz Robinett","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":139477494,"name":"Bad Apple","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1590940800000,"size":1,"copyrightId":1416618,"status":1,"picId":109951166982578395,"mark":0},"duration":282880,"copyrightId":1416618,"status":0,"alias":[],"rtype":0,"ftype":0,"mvid":0,"fee":8,"rUrl":null,"mark":794624},{"id":687506,"name":"Bad Apple!! feat. nomico","artists":[{"id":17423,"name":"のみこ","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":66494,"name":"EXSERENS","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1211644800000,"size":13,"copyrightId":743010,"status":1,"picId":109951166319416290,"mark":0},"duration":319426,"copyrightId":743010,"status":0,"alias":[],"rtype":0,"ftype":0,"mvid":290067,"fee":8,"rUrl":null,"mark":9007199255011456},{"id":22636739,"name":"Bad Apple!!","artists":[{"id":15345,"name":"上海アリス幻樂団","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":2075203,"name":"東方幻想郷 ~ Lotus Land Story","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":903024000000,"size":28,"copyrightId":-1,"status":1,"picId":676199651104974,"mark":0},"duration":169160,"copyrightId":663018,"status":0,"alias":[],"rtype":0,"ftype":0,"mvid":0,"fee":0,"rUrl":null,"mark":537001984},{"id":459925611,"name":"Bad Apple!!","artists":[{"id":13059968,"name":"Reol","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":35176532,"name":"東方ベストEDM","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1482940800007,"size":13,"copyrightId":0,"status":0,"picId":18729081069316352,"mark":0},"duration":302011,"copyrightId":663018,"status":0,"alias":["原曲：Bad Apple!!"],"rtype":0,"ftype":0,"mvid":0,"fee":0,"rUrl":null,"alias":["原曲：Bad Apple!!"],"mark":262144},{"id":34152128,"name":"Bad Apple","artists":[{"id":104700,"name":"Various Artists","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":3263927,"name":"最新热歌慢摇109","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1388505600004,"size":257,"copyrightId":0,"status":2,"picId":109951166361039007,"mark":0},"duration":217361,"copyrightId":0,"status":0,"alias":[],"rtype":0,"ftype":0,"mvid":0,"fee":0,"rUrl":null,"mark":786560},{"id":510051,"name":"Bad Apple!!","artists":[{"id":15345,"name":"上海アリス幻樂団","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":48429,"name":"幺乐団の歴史1","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1148140800000,"size":30,"copyrightId":0,"status":1,"picId":811439581299034,"mark":0},"duration":195186,"copyrightId":663018,"status":0,"alias":[],"rtype":0,"ftype":0,"mvid":0,"fee":0,"rUrl":null,"mark":9007199254872064},{"id":528478147,"name":"Bad Apple!!","artists":[{"id":17423,"name":"のみこ","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},{"id":190901,"name":"Masayoshi Minoshima","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":37099360,"name":"Bad Apple!! feat.nomico 10th Anniversary PHASE2","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1514476800000,"size":11,"copyrightId":0,"status":0,"picId":109951163100843000,"mark":0},"duration":316290,"copyrightId":663018,"status":0,"alias":[],"rtype":0,"ftype":0,"transNames":["坏苹果！！"],"mvid":5330539,"fee":0,"rUrl":null,"mark":262144},{"id":28996105,"name":"Bad Apple!!","artists":[{"id":16523,"name":"花たん","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p1.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":2975014,"name":"HANA TOHOBEST","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p1.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1408118400007,"size":13,"copyrightId":0,"status":0,"picId":6638851208564995,"mark":0},"duration":318000,"copyrightId":663018,"status":0,"alias":["原曲：Bad Apple!!"],"rtype":0,"ftype":0,"mvid":0,"fee":0,"rUrl":null,"alias":["原曲：Bad Apple!!"],"mark":9007199255003136},{"id":414691497,"name":"Bad Apple ?","artists":[{"id":21200,"name":"魂音泉","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p1.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null}],"album":{"id":34700769,"name":"Re:Raise TRIPLE","artist":{"id":0,"name":"","picUrl":null,"alias":[],"albumSize":0,"picId":0,"fansGroup":null,"img1v1Url":"http://p1.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg","img1v1":0,"trans":null},"publishTime":1462636800000,"size":8,"copyrightId":743010,"status":0,"picId":109951164943406609,"mark":0},"duration":333697,"copyrightId":743010,"status":0,"alias":["原曲:東方幻想郷 より Bad Apple!!"],"rtype":0,"ftype":0,"transNames":["Bad Apple? (feat. Romonosov?) - akarui_mirai Remix"],"mvid":0,"fee":8,"rUrl":null,"alias":["原曲:東方幻想郷 より Bad Apple!!"],"mark":270464}],"hasMore":true,"songCount":309},"code":200})");
  // std::clog<<j7.to_string()<<'\n';
  assert_equal(j7["result"]["songs"][3]["name"].to_string(), "Bad Apple!!");

  return 0;
}catch(xihale::json::exception &e){
  std::cerr<<e.what()<<std::endl;
  return 1;
}