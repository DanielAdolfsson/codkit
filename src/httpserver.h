#pragma once

#include <httplib.h>

namespace codkit::http {
    class HttpRequest {
    private:
        const httplib::Request *request;

    public:
        HttpRequest(const httplib::Request *request);
        void invalidate();
        [[nodiscard]] const std::string &get_body() const;
    };

    class HttpResponse {
    private:
        httplib::Response *response;

    public:
        HttpResponse(httplib::Response *response);
        void invalidate();
        [[nodiscard]] const std::string &get_body() const;
        void set_body(const std::string &value);
    };

    class HttpServer : private httplib::Server {
    public:
        void
        get(const std::string &pattern,
            const std::function<
                void(const std::shared_ptr<const HttpRequest> &, std::shared_ptr<HttpResponse> &)>
                &handler);
        void listen(const std::string &address, int port);
    };

    struct HttpContext {
        const httplib::Request &request;
        httplib::Response &response;
        const std::function<
            void(const std::shared_ptr<const HttpRequest> &, std::shared_ptr<HttpResponse> &)>
            &handler;
    };
} // namespace codkit::http
