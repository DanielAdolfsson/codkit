#pragma once

#include <httplib.h>

namespace codkit::http {
    struct HttpRequest {
        const std::string body;
    };

    struct HttpResponse {
        std::string body;
    };

    class HttpServer : private httplib::Server {
    public:
        void
        get(const std::string &pattern,
            const std::function<
                void(const HttpRequest &, const std::shared_ptr<HttpResponse> &)>
                &handler);
        void listen(const std::string &address, int port);
    };
} // namespace codkit::http
