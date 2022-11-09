#include "json.h"

namespace json {

      void Object::serialize(Printer &pf) const
      {$
          pf.print("{");
          pf.indent();
          pf.newline();

          std::unordered_map<std::string, Element>::const_iterator it;
          for (it = members_.cbegin(); it != members_.cend(); ++it) {

              String string {it->first};
              string.serialize(pf);
              pf.print(" : ");
              it->second.serialize(pf);

              if (next(it) != members_.cend()) {
                  pf.print(",");
                  pf.newline();
              }
          }

          pf.unindent();
          pf.newline();
          pf.print("}");
      }

      void Array::serialize(Printer &pf) const
      {$
          pf.print("[");

          std::vector<Element>::const_iterator it;
          for (it = elements_.cbegin(); it != elements_.cend(); ++it) {
              it->serialize(pf);
              if (next(it) != elements_.cend())
                  pf.print(", ");
          }

          pf.print("]");
      }

      void String::serialize(Printer &pf) const
      {$
          pf.print("\"%s\"", string_.c_str());
      }

      void Number::serialize(Printer &pf) const
      {$
          pf.print("%d", number_);
      }

      void Element::serialize(Printer &pf) const
      {$
          value_->serialize(pf);
      }
}


