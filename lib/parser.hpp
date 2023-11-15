// A simple json parser

#pragma once

#include <algorithm>
#include <cstddef>
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

  class exception: public std::exception{
  public:
    errors err;
    const char *what()const noexcept override{
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
    exception(const exception &)=default;
    exception(exception &&)=default;
    exception(errors _err):err(_err){}
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

    auto next(){
      while(pos<raw.length() && std::isspace(raw[pos])) ++pos;
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

    static constexpr char blank[]=" \t\r\n";
    bool is_blank(char ch){
      return std::ranges::find(blank, ch)!=std::end(blank);
    }

    char forward(){
      auto pos=this->pos;
      while(pos<raw.length() && is_blank(raw[pos])) ++pos;
      return raw[pos++];
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
          do{
            // get the string val
            auto key=get_block();
            key=key.substr(1, key.length()-2);
            if(next()!=':') throw exception(errors::invalid_json);
            val.insert({key, json(get_block())});
          }while(next()==',');
        }else if (first=='['){ // array
          j.val=vec();
          auto &val=std::get<vec>(j.val);
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
    
    json operator[](const std::string_view &key)const{ // for objects
      return std::get<umap>(val).at(key);
    }

    json operator[](const char *key)const{
      return std::get<umap>(val).at(key);
    }

    json operator[](const size_t &index)const{ // for arrays
      return std::get<vec>(val).at(index);
    }

    json operator[](const int &index)const{
      return std::get<vec>(val).at(index);
    }

    // Max type support: long long
    template <typename T> 
    requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
    operator T() const {
      size_t pos=0;
      auto const &raw=std::get<sv>(val);
      T v=std::stoll(raw.data(), &pos);
      if(pos!=raw.length())
        throw exception(not_number);
      return v;
    }

    template <typename T>
    requires std::is_floating_point_v<T>
    operator T() const {
      size_t pos=0;
      auto const &raw=std::get<sv>(val);
      T v=std::stod(raw.data(), &pos);
      if(pos!=raw.length())
        throw exception(not_number);
      return v;
    }

    operator std::string_view() const {
      auto const &raw=std::get<sv>(val);
      if(raw[0]!='"' || raw[raw.length()-1]!='"')
        throw exception(not_string);
      return raw.substr(1, raw.length()-2);
    }

    operator std::string() const {
      return std::string(std::string_view(*this));
    }

    operator bool() const {
      auto const &raw=std::get<sv>(val);
      if(raw=="true") return true;
      if(raw=="false") return false;
      throw exception(not_boolean);
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

    // TODO: operator==
    // TODO: to_string
  };
}
}
