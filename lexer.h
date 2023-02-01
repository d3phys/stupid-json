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
#include "../drop/drop.h"

namespace json
{

  class File final
  {
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

  struct TokenInfo final
  {
      std::size_t line;
      std::size_t line_position;

      TokenInfo() = default;

      TokenInfo(const char* start, const char* pos)
        : line {},
          line_position {}
      {
          const char* line_start {};
          for (line_start = start;
               start != pos;
               start++)
          {
              if (*start == '\n') {
                  line_start = start;
                  line++;
              }
          }

          line_position = static_cast<std::size_t>(start - line_start + 1);
          line++;
      }

      TokenInfo(const TokenInfo &other) = default;
      TokenInfo& operator=(const TokenInfo &other) = default;
  };

  class SyntaxError final
    : public std::runtime_error
  {
    public:
      using std::runtime_error::runtime_error;

      SyntaxError(const std::string& what, const TokenInfo& info)
        : std::runtime_error {
            drop::format("Config file:%ld:%ld: syntax error: %s\n", info.line, info.line_position, what.c_str())
          }
      {}
  };

  class SemanticError final
    : public std::runtime_error
  {
    public:
      using std::runtime_error::runtime_error;

      SemanticError(const std::string& what, const TokenInfo& info)
        : std::runtime_error {
            drop::format("config:%ld:%ld: semantic error: %s\n", info.line, info.line_position, what.c_str())
          }
      {}
  };

  class Lexer final
    : drop::noncopyable
  {
    private:
      const char* data_;
      const char* buf_;

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
      Lexer(const char *buf)
        : data_ {buf},
          buf_ {buf}
      {}

      char next() { return skip_ws(), *buf_; }
      bool next(char ch) { return skip_ws(), (*buf_ == ch); }

      TokenInfo get_token_info() const { return TokenInfo {data_, buf_}; }

      bool require(char ch)
      {
          const char* start = buf_;
          if (next(ch)) {
              buf_++;
              return true;
          }

          buf_ = start;
          throw SyntaxError {
              drop::format("there must be '%c' token", ch),
              get_token_info()
          };
      }

      int number()
      {
          char *end = nullptr;
          int number = static_cast<int>(std::strtol(buf_, &end, 0));
          if (end == buf_)
              throw SyntaxError {
                  drop::format("there must be a number"),
                  get_token_info()
              };

          buf_ = end;
          return number;
      }

      std::string token(char ch)
      {
          const char *token = buf_;
          if (!find(ch)) {
              buf_ = token;
              throw SyntaxError {
                  drop::format("there must be a string"),
                  get_token_info()
              };
          }

          return std::string {token, static_cast<size_t>(buf_ - token)};
      }
  };
}

#endif /* LEXER_H */
