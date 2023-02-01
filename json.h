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
#include "pf.h"
#include "../drop/drop.h"

#define $ fprintf(stderr, "%s: %d\n", __PRETTY_FUNCTION__, __LINE__);

namespace json {

  class Object;
  class Array;

  class Value
  {
    private:
      TokenInfo info_;

    public:
      virtual       int& number()       { throw SemanticError {"value must be number", info_}; }
      virtual const int& number() const { throw SemanticError {"value must be number", info_}; }

      virtual       std::string& string()       { throw SemanticError {"value must be string", info_}; }
      virtual const std::string& string() const { throw SemanticError {"value must be string", info_}; }

      virtual       Object& object()       { throw SemanticError {"value must be object", info_}; }
      virtual const Object& object() const { throw SemanticError {"value must be object", info_}; }

      virtual       Array& array()       { throw SemanticError {"value must be array", info_}; }
      virtual const Array& array() const { throw SemanticError {"value must be array", info_}; }

      virtual void serialize(Printer& pf) const = 0;

      Value() = default;
      Value(const TokenInfo& info)
        : info_ {info}
      {}

      Value(const Value& other) = default;
      Value& operator=(const Value& other) = default;

      const TokenInfo& info() const { return info_; }

      virtual ~Value() {}
  };

  class String final
    : public Value
  {
    private:
      std::string string_;

    public:
      String(const std::string& string)
        : string_ {string}
      {}

      String(Lexer& lex)
        : Value {lex.get_token_info()},
          string_ {}
      {
          lex.require('"');
          string_ = lex.token('"');
          lex.require('"');
      }

      virtual void serialize(Printer& pf) const override;

      virtual       std::string& string()       override { return string_; }
      virtual const std::string& string() const override { return string_; }
  };

  class Number final : public Value {
    public:
      enum Base {
          hex,
          decimal,
          octal,
      };

    private:
      int number_;
      Base base_;
    public:
      Number(int number, Base base = decimal)
        : number_ {number},
          base_ {base}
      {}

      Number(Lexer& lex)
        : Value {lex.get_token_info()},
          number_ {},
          base_ {decimal}
      {
          number_ = lex.number();
      }

            Base& base()       { return base_; }
      const Base& base() const { return base_; }

      virtual void serialize(Printer& pf) const override;

      virtual       int& number()       override { return number_; }
      virtual const int& number() const override { return number_; }
  };

  class Element;
  class Object final
    : public Value
  {
    private:
      std::unordered_map<std::string, Element> members_;

    public:
      Object() = default;
      Object(const std::string& path);
      Object(Lexer& lex);

      Element& operator()(const std::string& key)
      {
          assert(!members_.count(key));
          return members_[key];
      }

      virtual void serialize(Printer& pf) const override;
      void serialize(FILE *file) const
      {
          Printer pf {file};
          serialize(pf);
      }

            Element& operator[](const std::string& key)       { assert(members_.count(key)); return members_.at(key); }
      const Element& operator[](const std::string& key) const { assert(members_.count(key)); return members_.at(key); }

      virtual       Object& object()       override { return *this; }
      virtual const Object& object() const override { return *this; }

      bool contains(const std::string& key) const { return !!(members_.count(key)); }
  };

  class Array final : public Value {
    private:
      std::vector<Element> elements_;

    public:
      Array() = default;
      Array(Lexer& lex);

      virtual void serialize(Printer& pf) const override;

      Element& operator()() { return elements_.emplace_back(), elements_.back(); }

      Element& operator[](size_t key)
      {
          assert(key < elements_.size());
          return elements_.at(key);
      }

      const Element& operator[](size_t key) const
      {
          assert(key < elements_.size());
          return elements_.at(key);
      }

      virtual       Array& array()       override { return *this; }
      virtual const Array& array() const override { return *this; }

      const std::vector<Element>& elements() const { return elements_; }
      size_t size() const { return elements_.size(); }
  };

  class Element final
    : public Value
  {
    private:
      Value* value_;

    public:
      Element() = default;
      Element(Lexer& lex);

      Element& operator()()                       { return value_->array()(); }
      Element& operator()(const std::string& key) { return value_->object()(key); }

            Element& operator[](const std::string& key)       { return value_->object()[key]; }
      const Element& operator[](const std::string& key) const { return value_->object()[key]; }

            Element& operator[](size_t key)       { return value_->array()[key]; }
      const Element& operator[](size_t key) const { return value_->array()[key]; }

      Element& operator=(const int& number)
      {
          delete value_;
          value_ = new Number {number};
          return *this;
      }

      Element& operator=(const size_t& number)
      {
          delete value_;
          value_ = new Number {
              static_cast<int>(number)
          };
          return *this;
      }

      Element& operator=(const unsigned& number)
      {
          delete value_;
          value_ = new Number {
              static_cast<int>(number)
          };
          return *this;
      }

      Element& operator=(Number *number)
      {
          assert(number);
          delete value_;
          value_ = number;
          return *this;
      }

      Element& operator=(const std::string& string)
      {
          delete value_;
          value_ = new String {string};
          return *this;
      }

      Element& operator=(Object *object)
      {
          assert(object);
          delete value_;
          value_ = object;
          return *this;
      }

      Element& operator=(Array *array)
      {
          assert(array);
          delete value_;
          value_ = array;
          return *this;
      }

      /**
       * Delete assign operators for now
       * to prevent such situations:
       *
       *  {
       *    ...
       *    Element one {...};
       *    Element two {...};
       *    two = one;
       *    ...
       *    return two;
       *  } < Here 'one' will free both 'one' and 'two' values.
       *
       * Copying full elements tree is not cheap.
       * Assigning null to one's value is
       * a little bit strange behaviour.
       *
       * Assigning nullptr is a great solution
       * in combination with move semantics.
       */
      Element(const Element& element) = delete;
      Element& operator=(const Element& element) = delete;

      Element(Element&& other)
        : value_ {other.value_}
      {
          other.value_ = nullptr;
      }

      Element& operator=(Element&& other)
      {
          assert(!value_);
          delete value_;
          value_ = other.value_;
          other.value_ = nullptr;
          return *this;
      }

      ~Element() { delete value_; }

      void serialize(Printer& pf) const override;
      void serialize(FILE *file) const
      {
          Printer pf {file};
          serialize(pf);
      }

      virtual       int& number()       override { return value_->number(); }
      virtual const int& number() const override { return value_->number(); }

      virtual       Object& object()       override { return value_->object(); }
      virtual const Object& object() const override { return value_->object(); }

      virtual       Array& array()       override { return value_->array(); }
      virtual const Array& array() const override { return value_->array(); }

      virtual       std::string& string()       override { return value_->string(); }
      virtual const std::string& string() const override { return value_->string(); }
  };
}

#endif /* STUPID_JSON */
