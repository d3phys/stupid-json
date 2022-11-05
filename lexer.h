#ifndef LEXER_H
#define LEXER_H

#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <stdexcept>

namespace json {

  class Lexer {
    private:
      const char *buf_;

      void skip_ws()
      {
          while (*buf_ && std::isspace(*buf_))
              buf_++;
      }

      bool find(char ch)
      {
          while (*buf_ && *buf_ != ch)
              buf_++;

          return (*buf_ == ch);
      }

    public:
      class Exception : public std::exception {
        private:
          const char *where_;
          const char *what_;
        public:
          Exception(const char *what, const char *where)
            : where_ {where},
              what_ {what}
          {}

          virtual const char *what() const noexcept override { return what_; }
          const char *where() const noexcept { return where_; }
      };
      Lexer(const char *buf)
        : buf_ {buf}
      {}

      char next() { return skip_ws(), *buf_; }
      bool next(char ch) { return skip_ws(), (*buf_ == ch); }

      bool require(char ch)
      {
          if (next(ch)) {
              buf_++;
              return true;
          }

          throw Exception {"lexer: require error", buf_};
      }

      int number()
      {
          char *end = nullptr;
          int number = strtol(buf_, &end, 10);
          if (end == buf_)
              throw Exception {"lexer: number parse error", buf_};

          buf_ = end;
          return number;
      }

      std::string token(char ch)
      {
          const char *token = buf_;
          fprintf(stderr, "BUF: %s\n", buf_);
          if (!find(ch))
              throw Exception {"can't find ch for token", token};
          fprintf(stderr, "BUF: %s\n", buf_);

          return std::string {token, static_cast<size_t>(buf_ - token)};
      }
  };
}

#endif /* LEXER_H */
