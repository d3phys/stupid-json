#include "json.h"

namespace json {

  Array::Array(Lexer &lex)
    : elements_ {}
  {
      lex.require('[');
      if (!lex.next(']')) {
          do {
              Element *elem = new Element {lex};
              elements_.push_back(elem);
          } while (lex.next(',') && lex.require(','));
      }

      lex.require(']');
  }

  Object::Object(Lexer &lex)
    : members_ {}
  {
      try {

          lex.require('{');

          $
          if (!lex.next('}')) {
          $
              do {
          $
                  String name {lex};
          $
                  lex.require(':');
          $
                  members_[name.as_string()] = new Element {lex};
          $
              } while (lex.next(',') && lex.require(','));
          }

          $
          lex.require('}');
          $

      }
      catch (const json::Lexer::Exception &exc) {
          fprintf(stderr, "what: %s\n", exc.what());
          fprintf(stderr, "where: %s\n", exc.where());
          // Process exception
      }
      catch (const std::exception &exc) {
          fprintf(stderr, "what: %s\n", exc.what());
          // Process exception
      }
      catch (...) {
          assert(0);
      }

  }

  Array::~Array()
  {
      std::vector<Element *>::iterator it;
      for (it = elements_.begin(); it != elements_.end(); ++it)
          delete *it;
  }

  Object::~Object()
  {
      std::unordered_map<std::string, class Element *>::iterator it;
      for (it = members_.begin(); it != members_.end(); ++it)
          delete it->second;
  }

  Element::Element(Lexer &lex)
    : value_ {nullptr}
  {
      switch (lex.next()) {
        case '{':
          value_ = new Object {lex};
          return;
        case '[':
          value_ = new Array {lex};
          return;
        case '"':
          value_ = new String {lex};
          return;
        default:
          value_ = new Number {lex};
          return;
      }

      throw std::runtime_error {"fail to create element\n"};
  }

}

