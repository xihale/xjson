// A simple json parser

#pragma once

#include <algorithm>
#include <cstddef>
#include <format>
#include <functional>
#include <ranges>
#include <cctype>
#include <charconv>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace xihale{
namespace json{

  enum val_t{
    object,
    array,
    string,
    number,
    boolean,
    null,
  };

  // TODO: better error hints
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

  // pay attention to Dangling Reference
  class json{
  private:
    using sv=std::string_view;
    using umap=std::unordered_map<std::string_view, json>;
    using vec=std::vector<json>;
    using variant=std::variant<sv, umap, vec>;
    variant val;

  private:
    // help to parse
    json()=default;
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

      auto trim_left(std::string_view &raw){
        auto pos=raw.find_first_not_of(' ');
        if(pos!=std::string_view::npos) raw=raw.substr(pos, raw.length()-pos);
      }

      void parse(json &j){
        trim_left(raw);
        size_t pos=0;
        auto first=next();
        if(first!='{' && first!='[') // not object or array
          return void(j.val=raw);
        // object array string
        using ssize_t=unsigned char;
        
        if(first=='{'){ // object
          // string:val
          j.val=umap();
          auto &val=std::get<umap>(j.val);
          if(forward()=='}') return;
          do{
            // get the string val
            auto key=get_block();
            key=key.substr(1, key.length()-2);
            if(next()!=':') throw exception(errors::invalid_json, raw.substr(pos-10, std::max(raw.length()-pos, 20ul)));
            // get the val
            val.insert({key, json(get_block())});
          }while(next()==',');
        }else if (first=='['){ // array
          j.val=vec();
          auto &val=std::get<vec>(j.val);
          if(forward()==']') return;
          do{
            val.push_back(json(get_block()));
          }while(next()==',');
        }
      }
    };

  public:
    // json()=default;
    // json(const json &)=default;
    // json(json &&)=default;

    json(std::string_view raw){ // build from string
      parser(raw).parse(*this);
    }
    
    json& operator[](const std::string_view &key){ // for objects
      return std::get<umap>(val).at(key);
    }

    json& operator[](const char *key){
      return std::get<umap>(val).at(key);
    }

    json& operator[](const size_t &index){ // for arrays
      return std::get<vec>(val).at(index);
    }

    json& operator[](const int &index){
      return std::get<vec>(val).at(index);
    }

    // Max type support: long long
    template <typename T> 
    requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
    operator T() const {
      auto const &raw=std::get<sv>(val);
      // std::function<>
      T v=0;
      auto result = std::from_chars(raw.data(), raw.data()+raw.length(), v);
      if(result.ec!=std::errc() || result.ptr!=raw.data()+raw.length())
        throw exception(not_number, raw);
      return v;
    }

    template <typename T>
    requires std::is_floating_point_v<T>
    operator T() const {
      size_t pos=0;
      auto const &raw=std::get<sv>(val);
      T v=std::stod(raw.data(), &pos);
      if(pos!=raw.length())
        throw exception(not_number, raw);
      return v;
    }

    operator std::string_view() const {
      auto const &raw=std::get<sv>(val);
      if(!is_string())
        throw exception(not_string, raw);
      return raw.substr(1, raw.length()-2);
    }

    operator std::string() const {
      if(std::holds_alternative<umap>(val)){ // object to string
        std::string res="{";
        auto &raw=std::get<umap>(val);
        res.reserve(raw.size()*10);
        for(auto &i: raw)
          std::format_to(std::back_inserter(res), "\"{}\":{},", i.first, (i.second.is_array() || i.second.is_object())?i.second.operator std::string():std::get<sv>(i.second.val));
        res.pop_back(); // remove the last ,
        res+="}";
        return res;
      }else if(std::holds_alternative<vec>(val)){ // array to string
        std::string res="[";
        auto &raw=std::get<vec>(val);
        res.reserve(raw.size()*6);
        for(auto &i: raw)
          std::format_to(std::back_inserter(res), "{},", (i.is_array() || i.is_object())?i.operator std::string():std::get<sv>(i.val));
        res.pop_back();
        res+="]";
        return res;
      }
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
      std::string res;
      res.reserve(raw.length());
      while(pos<raw.length()){
        if(raw[pos]=='\\' && pos+1<raw.length()){
          res+=trans.at(raw[pos+1]);
          ++pos;
        }else
          res+=raw[pos];
        ++pos;
      }
      return res;
    }

    operator bool() const {
      auto const &raw=std::get<sv>(val);
      if(raw=="true") return true;
      if(raw=="false") return false;
      throw exception(not_boolean, raw);
    }

    bool is_null() const {
      auto const &raw=std::get<sv>(val);
      return raw=="null";
    }

    bool is_object() const {
      return val.index()==1;
    }

    bool is_array() const {
      return val.index()==2;
    }

    bool is_string() const {
      auto const &raw=std::get<sv>(val);
      return raw[0]=='"' && raw[raw.length()-1]=='"';
    }

    bool is_number(){
      return !is_object() && !is_array() && !is_string() && !is_null();
    }

    template <typename T>
    json& operator=(const T &other){
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
      std::get<umap>(this->val).insert({key, val});
      return *this;
    }

    json& insert(const json &val){
      if(!is_array())
        throw exception(not_array, std::string(*this));
      std::get<vec>(this->val).push_back(val);
      return *this;
    }

    auto to_string() const {
      return std::string(*this);
    }

    auto to_string_view() const {
      return std::string_view(*this);
    }
  };
}
}