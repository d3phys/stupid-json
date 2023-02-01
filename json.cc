#include "json.h"

namespace json {

  Array::Array(Lexer &lex)
    : Value {lex.get_token_info()},
      elements_ {}
  {$
      lex.require('[');

      if (!lex.next(']')) {
          do {
              elements_.push_back(Element {lex});
          }
          while (lex.next(',') && lex.require(','));
      }

      lex.require(']');
  }

  Object::Object(Lexer &lex)
    : Value {lex.get_token_info()},
      members_ {}
  {
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

  Element::Element(Lexer &lex)
    : Value {lex.get_token_info()},
      value_ {}
  {$
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
}
