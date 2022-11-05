#ifndef STUPID_JSON
#define STUPID_JSON

#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "lexer.h"

#define $ fprintf(stderr, "%s: %d\n", __PRETTY_FUNCTION__, __LINE__);
namespace json {

  class Object;
  class Array;

  class Value {
    public:
      virtual int as_number() { assert(0); }
      virtual std::string &as_string() { assert(0); }
      virtual Object &as_object() { assert(0); }
      virtual Array &as_array() { assert(0); }

      virtual ~Value() {}
  };

  class String : public Value {
    private:
      std::string string_;
    public:

      String(std::string &string)
        : string_ {string}
      {}

      String(Lexer &lex)
        : string_ {}
      {
          $
          lex.require('"');
          $
          string_ = lex.token('"');
          $
          lex.require('"');
          $
      }

      virtual std::string &as_string() override { return string_; }
      virtual ~String() {}
  };

  class Number : public Value {
    private:
      int number_;
    public:
      Number(int number)
        : number_ {number}
      {}

      Number(Lexer &lex)
        : number_ {0}
      {
          number_ = lex.number();
      }

      virtual int as_number() override { return number_; }
  };

  class Object : public Value {
    private:
      std::unordered_map<std::string, class Element *> members_;
    public:
      Object(const std::string &path);
      Object(Lexer &lex);

      Element &operator[](const std::string &key) { return assert(members_.count(key)), *members_[key]; }
      virtual Object &as_object() override { return *this; }
      virtual ~Object();
  };

  class Array : public Value {
    private:
      std::vector<Element *> elements_;
    public:
      Array(const std::string &path);
      Array(Lexer &lex);

      Element &operator[](size_t key) { return *elements_[key]; }
      virtual Array &as_array() override { return *this; }
      virtual ~Array();
  };

  class Element final {
    private:
      Value *value_;

    public:
      Element &operator[](const std::string &key) { return value_->as_object()[key]; }
      Element &operator[](size_t key) { return value_->as_array()[key]; }
      Element(Lexer &lex);

      Element &operator=(const Element &element) = delete;
      Element(const Element &element) = delete;

      ~Element() { delete value_; }

      explicit operator int() const { return value_->as_number(); }
      explicit operator std::string() const { return value_->as_string(); }
      explicit operator Object&() const { return value_->as_object(); }
      explicit operator Array&() const { return value_->as_array(); }
  };
}

#endif /* STUPID_JSON */
