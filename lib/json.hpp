// A simple json parser

#pragma once

#include <algorithm>
#include <cstddef>
#include <format>
#include <forward_list>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <ranges>
#include <cctype>
#include <charconv>
#include <concepts>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

namespace xihale{
namespace json{

  enum errors{
    not_number,
    not_boolean,
    not_null,
    not_object,
    not_array,
    not_string,
    not_found,
    invalid_json
  };

  class exception{
  public:
    errors err;
    std::string_view details;
    const char *why()const noexcept{
      switch(err){
        case not_number: return "not a number";
        case not_boolean: return "not a boolean";
        case not_null: return "not null";
        case not_object: return "not an object";
        case not_array: return "not an array";
        case not_string: return "not a string";
        case not_found: return "not found";
        case invalid_json: return "invalid json";
      }
    }
    const std::string what()const noexcept{
      return std::string(why()) + ": `" + std::string(details) +'`';
    }
    exception(const exception &)=default;
    exception(exception &&)=default;
    exception(errors _err, std::string_view _details=""):err(_err), details(_details){}
  };

  class json;

  typedef std::unordered_map<std::string, json> object_t;
  typedef std::vector<json> array_t;

  // Concepts
  // template<typename T>
    // concept Number=(std::is_integral_v<T> || std::is_floating_point_v<T>) && !std::is_same_v<T, bool>;
  template<typename T>
    concept Integer=std::is_integral_v<T> && !std::is_same_v<T, bool>;
  template<typename T>
    concept Float=std::is_floating_point_v<T> || Integer<T>;
  template<typename T>
    concept Boolean=std::is_same_v<T, bool>;
  template<typename T>
    concept Null=std::is_same_v<T, std::nullptr_t>;
  template<typename T>
    concept String=std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;
  template<typename T>
    concept Object=std::is_same_v<T, object_t>;
  template<typename T>
    concept Array=std::is_same_v<T, array_t>;

  namespace parser{
    static json parse(std::string_view &);
  };

  class json{
  private:
    using ll=long long;
    using nullptr_t=std::nullptr_t;
    using string_view=std::string_view;
    using string=std::string;
    using variant=std::variant<object_t, array_t, string, double, ll, bool, nullptr_t>;
    variant val;

  private:
    friend json parser::parse(std::string_view &);

    std::string_view get_raw() const {
      return std::get<string>(val).data();
    }

  public:
    json()=default;
    json(const json &)=default;
    json(json &&)=default;
    json &operator=(const json &)=default;
    json &operator=(json &&)=default;

    json(const std::initializer_list<std::pair<std::string_view, json>> &obj){
      this->val=object_t();
      auto &val=std::get<object_t>(this->val);
      for(auto &i:obj)
        val.insert({string(i.first), std::move(i.second)});
    }

    // TODO: other initialize_list

    template<Float T>
    json(const T &val){
      this->val=static_cast<double>(val);
    }

    template<Integer T>
    json(const T &val){
      this->val=static_cast<ll>(val);
    }

    json(std::string_view raw){ // build from string
      *this=parser::parse(raw);
    }
    json(const char *raw){
      string_view _raw=std::string_view(raw);
      *this=parser::parse(_raw);
    }
    json(const std::string &raw){
      string_view _raw=std::string_view(raw);
      *this=parser::parse(_raw);
    }
    json(std::string &&raw){
      string_view _raw=std::string_view(raw);
      *this=parser::parse(_raw);
    }

    json& operator[](const char *key){
      return std::get<object_t>(val).at(key);
    }

    json& operator[](const size_t &index){ // for arrays
      return std::get<array_t>(val).at(index);
    }

    json& operator[](const int &index){
      return std::get<array_t>(val).at(index);
    }

    template<typename T>
    T& get(){
      return std::get<T>(val);
    }

    template<typename T>
    const T& getc(){
      return std::get<T>(val);
    }

    template <typename T>
    requires std::is_floating_point_v<T>
    operator T() const {
      return static_cast<T>(std::get<double>(val));
    }

    template <typename T>
    requires std::is_integral_v<T>
    operator T() const {
      return static_cast<T>(std::get<ll>(val));
    }

    operator const object_t&() const {
      return std::get<object_t>(val);
    }

    operator const array_t&() const {
      return std::get<array_t>(val);
    }

    // explicit: prevent implicit conversion(std::string -> string_view)
    explicit operator std::string_view() const {
      return get_raw();
    }

    // TODO: try no use rescue
    operator std::string() const {
      std::string res;
      if(std::holds_alternative<object_t>(val)){ // object to string
        auto &raw=std::get<object_t>(val);
        res.reserve(raw.size()*10);
        res.push_back('{');
        for(auto &i: raw){
          std::format_to(std::back_inserter(res), "\"{}\":", i.first);
          if(i.second.is_string())
            std::format_to(std::back_inserter(res), "\"{}\",", i.second.get_raw());
          else
            std::format_to(std::back_inserter(res), "{},", std::string(i.second));
        }
        // res.pop_back(); // remove the last ,
        // res.push_back('}')
        if(res.back()==',') res.back()='}';
        else res.push_back('}');
      }else if(std::holds_alternative<array_t>(val)){ // array to string
        auto &raw=std::get<array_t>(val);
        res.reserve(raw.size()*6);
        res.push_back('[');
        for(auto &i: raw){
          if(i.is_string())
            std::format_to(std::back_inserter(res), "\"{}\",", i.get_raw());
          else
            std::format_to(std::back_inserter(res), "{},", std::string(i));
        }
        // res.pop_back();
        // res+="]";
        if(res.back()==',') res.back()=']';
        else res.push_back(']');
      }else if (is_string()){
        // others to string
        auto raw=std::string_view(*this);
        static const std::unordered_map<char, char> trans{
          {'\\', '\\'},
          {'"', '"'},
          {'\'', '\''},
          {'0', '\0'},
          {'b', '\b'},
          {'f', '\f'},
          {'n', '\n'},
          {'r', '\r'},
          {'t', '\t'},
        };
        size_t pos=0;
        res.reserve(raw.length());
        while(pos<raw.length()){
          if(raw[pos]=='\\' && pos+1<raw.length()){
            res+=trans.at(raw[pos+1]);
            ++pos;
          }else
            res+=raw[pos];
          ++pos;
        }
      }
      else{
        if(std::holds_alternative<double>(val))
          res=std::to_string(std::get<double>(val));
        else if(std::holds_alternative<ll>(val))
          res=std::to_string(std::get<ll>(val));
        else if(std::holds_alternative<bool>(val))
          res=std::get<bool>(val)?"true":"false";
        else if(std::holds_alternative<nullptr_t>(val))
          res="null";
      }
      return res;
    }

    operator bool() const {
      return std::get<bool>(val);
    }

    bool is_null() const {
      return std::holds_alternative<nullptr_t>(val);
    }

    bool is_object() const {
      return std::holds_alternative<object_t>(val);
    }

    bool is_array() const {
      return std::holds_alternative<array_t>(val);
    }

    bool is_string() const {
      return std::holds_alternative<std::string>(val);
    }

    bool is_number(){
      return std::holds_alternative<double>(val) || std::holds_alternative<ll>(val);
    }

    template <typename T>
    json& operator=(T other){
      val=std::forward<T>(other);
      return *this;
    }

    // template <typename T>
    // bool operator==(const T &other) const {
    //   return std::string(*this)==std::to_string(other);
    // }

    json& insert(const std::string &key, const json &val){
      if(!is_object())
        throw exception(not_object, std::string(*this));
      std::get<object_t>(this->val).insert({key, val});
      return *this;
    }

    json& insert(const json &val){
      if(!is_array())
        throw exception(not_array, std::string(*this));
      std::get<array_t>(this->val).push_back(val);
      return *this;
    }

    auto to_string() const {
      return std::string(*this);
    }

    auto to_string_view() const {
      return std::string_view(*this);
    }

    auto &get_object() {
      return std::get<object_t>(val);
    }
    auto &get_array() {
      return std::get<array_t>(val);
    }

    auto &get_const_object() const {
      return std::get<object_t>(val);
    }
    auto &get_const_array() const {
      return std::get<array_t>(val);
    }
  };

  namespace parser{

    using std::string_view;
    using ll=long long;
    static auto &npos=string_view::npos;
    // static void trim_left(string_view &raw){
    //   auto bpos=raw.find_first_not_of(' ');
    //   if(bpos!=npos) raw.remove_prefix(bpos);
    // }

    // fast judge white space
    static constexpr bool is_blank[256]{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    static char next(string_view &raw){
      while(is_blank[static_cast<uint8_t>(raw.front())]) raw.remove_prefix(1);
      raw.remove_prefix(1);
      return *(raw.data()-1);
    }

    static char forward(string_view &raw){
      bool vis=is_blank[static_cast<uint8_t>(raw.front())];
      while(is_blank[static_cast<uint8_t>(raw.front())]) raw.remove_prefix(1);
      return *(raw.data()-vis);
    }


    // static bool forward(string_view &raw, const string_view &judge){
    //   return raw.starts_with(judge.substr(1));
    // }

    static void skip_until(string_view &raw, const char &judge){
      while(next(raw)!=judge);
    }

    static void skip(string_view &raw){
      char ch;
      while(!raw.empty() && (ch=raw[0])!=',' && ch!='}' && ch!=']') raw.remove_prefix(1);
    }

    static json parse(string_view &raw){
      json j;
      auto &v=j.val;
      auto begin=next(raw);
      if(begin=='{'){
        char ch;
        v=object_t{};
        auto &o=std::get<object_t>(v);
        while((ch=next(raw))=='"'){
          auto bpos=raw.begin();
          skip_until(raw, '"');
          auto key=std::string(bpos, raw.begin()-1);
          next(raw); // :
          o.insert({key, parse(raw)});
          if(forward(raw)==',') next(raw);
        }
      }else if(begin=='['){
        // char ch;
        v=array_t{};
        auto &a=std::get<array_t>(v);
        a.reserve(4);
        // if(forward(raw)==']'){ next(raw); return j; }
        // raw=string_view(raw.begin()-1, raw.end());
        while(forward(raw)!=']'){
          a.push_back(parse(raw));
          if(forward(raw)==',') next(raw);
        }
        next(raw); // ]
      }else if(begin=='"'){
        auto bpos=raw.begin();
        skip_until(raw, '"');
        v=std::string(bpos, raw.begin()-1);
      }else if(begin=='t'){ // true
        v=true, skip(raw);
      }else if(begin=='f'){ // false
        v=false, skip(raw);
      }else if(begin=='n'){ // null
        v=nullptr, skip(raw);
      }else{ // number
        std::from_chars_result res;
        auto bpos=raw.begin()-1;
        skip(raw);
        v=(double)0;
        res=std::from_chars(bpos, raw.begin(), (double&)v);
        ll i=std::get<double>(v);
        if(std::get<double>(v)-i<1e-6) v=i;
        // if(res.ec!=std::errc() || res.ptr!=raw.begin()){
        //   v=string_view(bpos, raw.begin()); // regarded as String
        // }
      }
      return j;
    }
  }
}
}
