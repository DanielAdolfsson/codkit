#pragma once

#include "httplib.h"
#define SOL_USING_CXX_LUA 1
#include "sol/forward.hpp"

namespace codkit::lua {
    class HttpServerRequest {
    private:
        std::string _body;

    public:
        explicit HttpServerRequest(const httplib::Request &request);
        [[nodiscard]] const std::string &GetBody() const;
    };

    using HttpServerRequestPtr = std::unique_ptr<HttpServerRequest>;

    class HttpServerResponse
        : public std::enable_shared_from_this<HttpServerResponse> {
    private:
        std::string _body;

    public:
        void Commit(httplib::Response &response) const;
        std::shared_ptr<HttpServerResponse> WithBody(const sol::object &body);
    };

    using HttpServerResponsePtr = std::shared_ptr<HttpServerResponse>;

    class HttpServer : private httplib::Server {
    public:
        void
        Get(const std::string &pattern,
            const std::function<HttpServerResponsePtr(HttpServerRequestPtr)>
                &handler);
        void Mount(const std::string &mountPoint, const std::string &path);

        void Listen(int port);
        void Listen(int port, const std::string &host);
    };
} // namespace codkit::lua
