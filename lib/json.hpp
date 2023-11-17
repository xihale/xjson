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

  typedef std::unordered_map<std::string_view, json> object_t;
  typedef std::vector<json> array_t;

  class json{
  private:
    using ull=unsigned long long;
    using ll=long long;
    using nullptr_t=std::nullptr_t;
    using sv=std::string_view;
    using variant=std::variant<sv, object_t, array_t, std::string, double, bool, ull, ll, nullptr_t>;
    variant val;

  private:
  class parser{
  private:
    std::string_view raw;
    size_t pos=0;
  public:
    parser(const std::string_view &raw):raw(raw){}

    static constexpr char blank[]=" \t\r\n";
    bool is_blank(char ch){
      return std::ranges::find(blank, ch)!=std::end(blank);
    }

    auto next(){
      while(pos<raw.length() && is_blank(raw[pos])) ++pos;
      return raw[pos++];
    };
    
    static constexpr char skip_v[3][3]={{'{','}'}, {'[',']'}, {'"','"'}};
    auto get_skip(const auto &ch){
      ssize_t k;
      for(k=0;k<3;++k)
        if(ch==skip_v[k][0]) break;
      return k;
    };

    void skip(const auto &k){
      char ch;
      ssize_t _k;
      while((ch=next())!=skip_v[k][1]){
        // check is in skip_v
        if((_k=get_skip(ch))<3) skip(_k);
        if(k==2 && ch=='\\') next();
      }
    };


    char forward(){
      next();
      return raw[--pos];
    }

    auto get_block() ->sv {
      auto k=get_skip(next());
      size_t begin=pos-1;
      if(k<3) skip(k);
      else{
        // next until '}', ']' or ','
        char ch;
        while((ch=forward())!='}' && ch!=']' && ch!=',') next();
        return raw.substr(begin, pos-begin);
      }
      return raw.substr(begin, pos-begin);
    };

      auto trim(std::string_view &raw){
        auto bpos=raw.find_first_not_of(' ');
        auto epos=raw.find_last_not_of(' ');
        if(bpos!=std::string_view::npos) raw=raw.substr(bpos, epos-bpos+1);
      }

      void parse(json &j){
        trim(raw);
        size_t pos=0;
        auto first=next();
        if(first!='{' && first!='['){ // not object or array
          // judges
          // null
          if(raw=="null") j.val=nullptr_t();
          // number
          else if(raw=="true") j.val=true;
          else if(raw=="false") j.val=false;
          else if(raw[0]=='"' && raw[raw.length()-1]=='"')
            j.val=raw.substr(1, raw.length()-2);
          else{ // number
            // ll
            std::from_chars_result res;
            if(raw.find('.')!=std::string_view::npos) // double
              res=j.turn_to<double>(raw);
            else if(raw[0]=='-') // ll
              res=j.turn_to<ll>(raw);
            else // ull
              res=j.turn_to<ull>(raw);
            if(res.ec!=std::errc() || res.ptr!=raw.data()+raw.length()){
              // throw exception(errors::invalid_json, raw.substr(pos-10, std::max(raw.length()-pos, 20ul)));
              // maybe it's a string
              j.val=raw;
            }
            
          }
          return;
        }
        // object array string
        using ssize_t=unsigned char;
        
        if(first=='{'){ // object
          // string:val
          j.val=object_t();
          auto &val=std::get<object_t>(j.val);
          if(forward()=='}') return;
          do{
            // get the string val
            auto key=get_block();
            key=key.substr(1, key.length()-2);
            if(next()!=':') throw exception(errors::invalid_json, raw.substr(pos-10, std::max(raw.length()-pos, 20ul)));
            // get the val
            val.insert({key, std::move(json(get_block()))});
          }while(next()==',');
        }else if (first=='['){ // array
          j.val=array_t();
          auto &val=std::get<array_t>(j.val);
          if(forward()==']') return;
          do{
            val.push_back(std::move(json(get_block())));
          }while(next()==',');
        }
      }
    };

    template<typename T>
    std::from_chars_result turn_to(const sv &raw){
      T v;
      auto res=std::from_chars(raw.data(), raw.data()+raw.length(), v);
      val=v;
      return res;
    }

    std::string_view get_raw() const {
      if(std::holds_alternative<sv>(val))
        return std::get<sv>(val);
      return std::get<std::string>(val).data();
    }

  public:
    json()=default;

    json(const std::initializer_list<std::pair<std::string_view, json>> &obj){
      this->val=object_t();
      auto &val=std::get<object_t>(this->val);
      for(auto &i:obj)
        val.insert({i.first, std::move(i.second)});
    }

    template<typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    json(const T &val){
      *this=std::move(json(std::to_string(val)));
    }

    json(std::string_view raw){ // build from string
      parser(raw).parse(*this);
    }
    json(const char *raw){
      parser(raw).parse(*this);
    }
    json(const std::string &raw){
      parser(raw).parse(*this);
    }
    json(std::string &&raw){
      parser(std::forward<std::string&&>(raw)).parse(*this);
    }
    
    json& operator[](const std::string_view &key){ // for objects
      return std::get<object_t>(val).at(key);
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

    template <typename T> 
    requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
    operator T() const {
      if(std::holds_alternative<ll>(val))
        return std::get<ll>(val);
      return std::get<ull>(val);
    }

    template <typename T>
    requires std::is_floating_point_v<T>
    operator T() const {
      return std::get<double>(val);
    }

    operator  const object_t&() const {
      return std::get<object_t>(val);
    }

    operator const array_t&() const {
      return std::get<array_t>(val);
    }

    // explicit: prevent implicit conversion(std::string -> string_view)
    explicit operator std::string_view() const {
      return get_raw();
    }

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
        res.back()='}';
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
        res.back()=']';
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
        auto func=[&res](const decltype(val) &val){
          using T=std::decay_t<decltype(val)>;
          if constexpr(std::is_same_v<T, double>)
            res=std::to_string(std::get<double>(val));
          else if constexpr(std::is_same_v<T, bool>)
            res=std::get<bool>(val)?"true":"false";
          else if constexpr(std::is_same_v<T, nullptr_t>)
            res="null";
          else if(std::holds_alternative<ull>(val))
            res=std::to_string(std::get<ull>(val));
          else
            res=std::to_string(std::get<ll>(val));
        };
        std::visit(func, val);
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
      return std::holds_alternative<std::string_view>(val) || std::holds_alternative<std::string>(val);
    }

    bool is_number(){
      return std::holds_alternative<double>(val) || std::holds_alternative<ull>(val) || std::holds_alternative<ll>(val);
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

    json& insert(const std::string_view &key, const json &val){
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

    auto get_object(){
      return std::get<object_t>(val);
    }
    auto get_array(){
      return std::get<array_t>(val);
    }
  };
}
}
