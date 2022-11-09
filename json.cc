#include "json.h"

namespace json {

  Array::Array(Lexer &lex)
    : elements_ {}
  {$
      try {
          lex.require('[');

          if (!lex.next(']')) {
              do {
                  elements_.push_back(Element {lex});
              } while (lex.next(',') && lex.require(','));
          }

          lex.require(']');
      }
      catch (...) {
          throw;
      }
  }

  Object::Object(Lexer &lex)
    : members_ {}
  {$
      try {
          lex.require('{');

          if (!lex.next('}')) {
              do {
                  String name {lex};
                  lex.require(':');
                  members_[name.string()] = Element {lex};
              } while (lex.next(',') && lex.require(','));
          }

          lex.require('}');
      }
      catch (const json::Lexer::Exception &exc) {
          fprintf(stderr, "what: %s\n", exc.what());
          fprintf(stderr, "where: %s\n", exc.where());
          throw;
      }
      catch (const std::exception &exc) {
          fprintf(stderr, "what: %s\n", exc.what());
          throw;
      }
      catch (...) {
          throw;
      }
  }

  Element::Element(Lexer &lex)
    : value_ {nullptr}
  {$
      try {
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
      }
      catch (...) {
          throw;
      }
  }
}
