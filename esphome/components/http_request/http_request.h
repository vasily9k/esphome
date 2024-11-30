#pragma once

#include <list>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "esphome/components/json/json_util.h"
#include "esphome/core/application.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace http_request {

struct Header {
  const char *name;
  const char *value;
};

// Some common HTTP status codes
enum HttpStatus {
  HTTP_STATUS_OK = 200,
  HTTP_STATUS_NO_CONTENT = 204,
  HTTP_STATUS_PARTIAL_CONTENT = 206,

  /* 3xx - Redirection */
  HTTP_STATUS_MULTIPLE_CHOICES = 300,
  HTTP_STATUS_MOVED_PERMANENTLY = 301,
  HTTP_STATUS_FOUND = 302,
  HTTP_STATUS_SEE_OTHER = 303,
  HTTP_STATUS_NOT_MODIFIED = 304,
  HTTP_STATUS_TEMPORARY_REDIRECT = 307,
  HTTP_STATUS_PERMANENT_REDIRECT = 308,

  /* 4XX - CLIENT ERROR */
  HTTP_STATUS_BAD_REQUEST = 400,
  HTTP_STATUS_UNAUTHORIZED = 401,
  HTTP_STATUS_FORBIDDEN = 403,
  HTTP_STATUS_NOT_FOUND = 404,
  HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
  HTTP_STATUS_NOT_ACCEPTABLE = 406,
  HTTP_STATUS_LENGTH_REQUIRED = 411,

  /* 5xx - Server Error */
  HTTP_STATUS_INTERNAL_ERROR = 500
};

/**
 * @brief Returns true if the HTTP status code is a redirect.
 *
 * @param status the HTTP status code to check
 * @return true if the status code is a redirect, false otherwise
 */
inline bool is_redirect(int const status) {
  switch (status) {
    case HTTP_STATUS_MOVED_PERMANENTLY:
    case HTTP_STATUS_FOUND:
    case HTTP_STATUS_SEE_OTHER:
    case HTTP_STATUS_TEMPORARY_REDIRECT:
    case HTTP_STATUS_PERMANENT_REDIRECT:
      return true;
    default:
      return false;
  }
}

/**
 * @brief Checks if the given HTTP status code indicates a successful request.
 *
 * A successful request is one where the status code is in the range 200-299
 *
 * @param status the HTTP status code to check
 * @return true if the status code indicates a successful request, false otherwise
 */
inline bool is_success(int const status) { return status >= HTTP_STATUS_OK && status < HTTP_STATUS_MULTIPLE_CHOICES; }

class HttpRequestComponent;

class HttpContainer : public Parented<HttpRequestComponent> {
 public:
  virtual ~HttpContainer() = default;
  size_t content_length;
  int status_code;
  uint32_t duration_ms;

  virtual int read(uint8_t *buf, size_t max_len) = 0;
  virtual void end() = 0;

  void set_secure(bool secure) { this->secure_ = secure; }

  size_t get_bytes_read() const { return this->bytes_read_; }

 protected:
  size_t bytes_read_{0};
  bool secure_{false};
};

class HttpRequestResponseTrigger : public Trigger<std::shared_ptr<HttpContainer>, std::string &> {
 public:
  void process(std::shared_ptr<HttpContainer> container, std::string &response_body) {
    this->trigger(std::move(container), response_body);
  }
};

class HttpRequestComponent : public Component {
 public:
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  void set_useragent(const char *useragent) { this->useragent_ = useragent; }
  void set_timeout(uint16_t timeout) { this->timeout_ = timeout; }
  void set_watchdog_timeout(uint32_t watchdog_timeout) { this->watchdog_timeout_ = watchdog_timeout; }
  uint32_t get_watchdog_timeout() const { return this->watchdog_timeout_; }
  void set_follow_redirects(bool follow_redirects) { this->follow_redirects_ = follow_redirects; }
  void set_redirect_limit(uint16_t limit) { this->redirect_limit_ = limit; }

  std::shared_ptr<HttpContainer> get(std::string url) { return this->start(std::move(url), "GET", "", {}); }
  std::shared_ptr<HttpContainer> get(std::string url, std::list<Header> headers) {
    return this->start(std::move(url), "GET", "", std::move(headers));
  }
  std::shared_ptr<HttpContainer> post(std::string url, std::string body) {
    return this->start(std::move(url), "POST", std::move(body), {});
  }
  std::shared_ptr<HttpContainer> post(std::string url, std::string body, std::list<Header> headers) {
    return this->start(std::move(url), "POST", std::move(body), std::move(headers));
  }

  virtual std::shared_ptr<HttpContainer> start(std::string url, std::string method, std::string body,
                                               std::list<Header> headers) = 0;

 protected:
  const char *useragent_{nullptr};
  bool follow_redirects_{};
  uint16_t redirect_limit_{};
  uint16_t timeout_{4500};
  uint32_t watchdog_timeout_{0};
};

template<typename... Ts> class HttpRequestSendAction : public Action<Ts...> {
 public:
  HttpRequestSendAction(HttpRequestComponent *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, url)
  TEMPLATABLE_VALUE(const char *, method)
  TEMPLATABLE_VALUE(std::string, body)
  TEMPLATABLE_VALUE(bool, capture_response)

  void add_header(const char *key, TemplatableValue<const char *, Ts...> value) { this->headers_.insert({key, value}); }

  void add_json(const char *key, TemplatableValue<std::string, Ts...> value) { this->json_.insert({key, value}); }

  void set_json(std::function<void(Ts..., JsonObject)> json_func) { this->json_func_ = json_func; }

  void register_response_trigger(HttpRequestResponseTrigger *trigger) { this->response_triggers_.push_back(trigger); }

  void register_error_trigger(Trigger<> *trigger) { this->error_triggers_.push_back(trigger); }

  void set_max_response_buffer_size(size_t max_response_buffer_size) {
    this->max_response_buffer_size_ = max_response_buffer_size;
  }

  void play(Ts... x) override {
    std::string body;
    if (this->body_.has_value()) {
      body = this->body_.value(x...);
    }
    if (!this->json_.empty()) {
      auto f = std::bind(&HttpRequestSendAction<Ts...>::encode_json_, this, x..., std::placeholders::_1);
      body = json::build_json(f);
    }
    if (this->json_func_ != nullptr) {
      auto f = std::bind(&HttpRequestSendAction<Ts...>::encode_json_func_, this, x..., std::placeholders::_1);
      body = json::build_json(f);
    }
    std::list<Header> headers;
    for (const auto &item : this->headers_) {
      auto val = item.second;
      Header header;
      header.name = item.first;
      header.value = val.value(x...);
      headers.push_back(header);
    }

    auto container = this->parent_->start(this->url_.value(x...), this->method_.value(x...), body, headers);

    if (container == nullptr) {
      for (auto *trigger : this->error_triggers_)
        trigger->trigger();
      return;
    }

    size_t content_length = container->content_length;
    size_t max_length = std::min(content_length, this->max_response_buffer_size_);
    // https://github.com/vasily9k get base code from Mike Ford https://github.com/HLFCode/esphome
    // https://github.com/esphome/esphome/commit/a5dca6c8d30be7c9d2550223112a2f8a56166ca7#diff-1cf5bd82614da7ca2744ef3946b1ab6842d7652822ba1f00fc5c34ad6abd94bcR201

    // Returned header Content_Length = -1 is no helpful so we need
    // to keep track of read bytes in the body rather than assume we need to read Content_Length bytes
    bool invalid_content_length = (int) content_length < 0;
    if (invalid_content_length) {
      max_length = this->max_response_buffer_size_;
      ESP_LOGD("v9k", "max_length: %d", max_length);
    }

    std::string response_body;
    if (this->capture_response_.value(x...)) {
      ExternalRAMAllocator<uint8_t> allocator(ExternalRAMAllocator<uint8_t>::ALLOW_FAILURE);
      uint8_t *buf = allocator.allocate(max_length);
      if (buf != nullptr) {
        size_t read_index = 0;
        // Prevent loop getting stuck
        // 'read' will not increment if there are no more bytes to read
        size_t last_read_index = -1;
        while (container->get_bytes_read() < max_length && read_index != last_read_index) {
          last_read_index = read_index;
          int read = container->read(buf + read_index, std::min<size_t>(max_length - read_index, 512));
          App.feed_wdt();
          yield();
          ESP_LOGD("v9k", "read: %d, read_index: %d, last_read_index: %d",
            read, read_index, last_read_index);
          // Detect an attempt to read backwards (negative 'read')
          if (read < 0) break;
          read_index += read;
        }

        response_body.reserve(read_index);
        response_body.assign((char *) buf, read_index);
        allocator.deallocate(buf, max_length);
      }
    }
    // TODO: understand why response body has two leading bytes (ascii encoded length)
    // Keep trying to understand
    size_t body_len = response_body.length();
    ESP_LOGD("v9k", "response_body lenth before erase: %d", body_len);
    for (size_t i = 0; i < body_len; i++) {
        if (i < 20 || i + 20 > body_len) {
          char c = response_body[i];
          char hex_buffer[5];
          sprintf(hex_buffer, "0x%02X", static_cast<unsigned char>(c));
          ESP_LOGD("before", "№%D. Character: '%c' Hex: %s", i, c, hex_buffer);
          App.feed_wdt();
          yield();
        }
    }
    // As result: response body has redundant 4-6 leading bytes (L1, L2, L3*, L4*, 0x0D, 0x0A)
    // and 7 trailing bytes (0x0D, 0x0A, '0', 0x0D, 0x0A, 0x0D, 0x0A)
    // L1-L4 are from 2 to 4 characters indicating the length of the content in HEX

    if (invalid_content_length) {
      // trying to find the first LF into body
      size_t lf_position = response_body.find('\n');
      if (lf_position == std::string::npos) {
          ESP_LOGE("Tag", "Can't find 'LF' symbol");
      }

      if (response_body.length() > 2) {
        // strip (lf_position + 1) leading and 7 trailing bytes
        response_body.erase(0, lf_position + 1);
        response_body.erase(response_body.length() - 7, 7);
      }
      container->content_length = response_body.length();
    }
    // result check

    body_len = response_body.length();

    //ESP_LOGD("v9k", "response_body after erase: %s", response_body.c_str());
    for (size_t i = 0; i < body_len; i++) {
        char c = response_body[i];
        if (i < 20 || i + 20 > body_len) {
          char hex_buffer[5];
          sprintf(hex_buffer, "0x%02X", static_cast<unsigned char>(c));
          ESP_LOGD("after", "№%D. Character: '%c' Hex: %s", i, c, hex_buffer);
          App.feed_wdt();
          yield();
        }
    }
    if (invalid_content_length) {
      ESP_LOGD("v9k", "content length was corrected to %d", body_len);
    }

    if (this->response_triggers_.size() == 1) {
      // if there is only one trigger, no need to copy the response body
      this->response_triggers_[0]->process(container, response_body);
    } else {
      for (auto *trigger : this->response_triggers_) {
        // with multiple triggers, pass a copy of the response body to each
        // one so that modifications made in one trigger are not visible to
        // the others
        auto response_body_copy = std::string(response_body);
        trigger->process(container, response_body_copy);
      }
    }
    container->end();
  }

 protected:
  void encode_json_(Ts... x, JsonObject root) {
    for (const auto &item : this->json_) {
      auto val = item.second;
      root[item.first] = val.value(x...);
    }
  }
  void encode_json_func_(Ts... x, JsonObject root) { this->json_func_(x..., root); }
  HttpRequestComponent *parent_;
  std::map<const char *, TemplatableValue<const char *, Ts...>> headers_{};
  std::map<const char *, TemplatableValue<std::string, Ts...>> json_{};
  std::function<void(Ts..., JsonObject)> json_func_{nullptr};
  std::vector<HttpRequestResponseTrigger *> response_triggers_{};
  std::vector<Trigger<> *> error_triggers_{};

  size_t max_response_buffer_size_{SIZE_MAX};
};

}  // namespace http_request
}  // namespace esphome