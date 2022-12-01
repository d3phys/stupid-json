#ifndef LEXER_H
#define LEXER_H

#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <exception>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace json {
  class File final {
    private:
      std::string name_;
      int fd_;
      size_t size_;
      char *buffer_;

    public:
      File(const std::string &name)
        : name_ {name},
          fd_ {0},
          size_ {0}
      {
          struct stat st {};
          if (stat(name_.c_str(), &st) == -1)
              throw std::runtime_error {strerror(errno)};

          fd_ = open(name_.c_str(), O_RDONLY);
          if (fd_ == -1)
              throw std::runtime_error {strerror(errno)};

          size_ = static_cast<size_t>(st.st_size);
          buffer_ = new char[size_ + 1];
          memset(buffer_, 0, size_ + 1);

          if (read(fd_, buffer_, size_) == -1)
              throw std::runtime_error {strerror(errno)};
      }

      ~File()
      {
          delete[] buffer_;
          close(fd_);
      }

      char *data() { return buffer_; }
  };

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

          fprintf(stderr, "lexer: REQUIRE ERROR: %c\n", ch);
          throw Exception {"lexer: require error", buf_};
      }

      int number()
      {
          char *end = nullptr;
          int number = strtol(buf_, &end, 0);
          if (end == buf_)
              throw Exception {"lexer: number parse error", buf_};

          buf_ = end;
          return number;
      }

      std::string token(char ch)
      {
          const char *token = buf_;
          if (!find(ch))
              throw Exception {"can't find ch for token", token};

          return std::string {token, static_cast<size_t>(buf_ - token)};
      }
  };
}

#endif /* LEXER_H */
