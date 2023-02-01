#ifndef PRINT_FORMATTER_H
#define PRINT_FORMATTER_H

#include <cassert>
#include <stdio.h>
#include <stdarg.h>

namespace json {
  class Printer {
    private:
      const int indent_size_;
      int indent_;
      FILE *file_;

    public:
      Printer(FILE *file, int indent = 4)
        : indent_size_ {indent},
          indent_ {0},
          file_ {file}
      {}

      void newline() { fprintf (file_, "\n%*c", indent_, ' '); }

      void print(const char *fmt, ...)
      {
          assert(fmt);
          va_list args;
          va_start(args, fmt);
          vfprintf(file_, fmt, args);
          va_end(args);
      }

      void   indent() { indent_ += indent_size_; }
      void unindent() { indent_ -= indent_size_; }
  };
}

#endif /* PRINT_FORMATTER_H */
