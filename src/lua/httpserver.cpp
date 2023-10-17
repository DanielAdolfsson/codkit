#include "httpserver.h"
#include "../detours.h"
#include "../logging.h"

#define SOL_USING_CXX_LUA 1
#include "nlohmann/json.hpp"
#include "sol/sol.hpp"

namespace codkit::lua {
    nlohmann::json ToJson(const sol::object &object) {
        switch (object.get_type()) {
        case sol::type::lua_nil:
            return nullptr;
        case sol::type::string:
            return object.as<std::string>();
        case sol::type::number:
            return object.as<std::float_t>();
        case sol::type::table: {
            nlohmann::json result({});

            object.as<sol::table>().for_each([&](sol::object const &key,
                                                 sol::object const &value) {
                logging::Logger l{};
                l << "entry ";
                auto sKey = key.as<std::optional<std::string>>();
                if (sKey) {
                    l << "(is key string) " << sKey.value();
                    result[sKey.value()] = ToJson(value);
                }
            });

            return result;
        }
        default:
            return "???";
        }
    }

    HttpServerRequest::HttpServerRequest(const httplib::Request &request)
        : _body(request.body) {
    }

    const std::string &HttpServerRequest::GetBody() const {
        return _body;
    }

    void HttpServerResponse::Commit(httplib::Response &response) const {
        response.body = _body;
    }

    std::shared_ptr<HttpServerResponse>
    HttpServerResponse::WithBody(const sol::object &body) {
        if (body.is<std::string>()) {
            _body = body.as<std::string>();
        } else {
            _body = ToJson(body).dump();
        }
        return shared_from_this();
    }

    void HttpServer::Listen(int port, const std::string &host) {
        std::thread{[this, host, port]() {
            httplib::Server::listen(host, port, 0);
        }}.detach();
    }

    void HttpServer::Listen(int port) {
        std::thread{[this, port]() {
            httplib::Server::listen("0.0.0.0", port, 0);
        }}.detach();
    }

    void
    HttpServer::Mount(const std::string &mountPoint, const std::string &path) {
        set_mount_point(mountPoint, path);
    }

    void HttpServer::Get(
        const std::string &pattern,
        const std::function<HttpServerResponsePtr(HttpServerRequestPtr)>
            &handler
    ) {
        httplib::Server::Get(
            pattern,
            [=](const httplib::Request &request, httplib::Response &response) {
                detours::Enqueue([&]() {
                    auto httpResponse =
                        handler(std::make_unique<HttpServerRequest>(request));
                    httpResponse->Commit(response);
                    return 0;
                });
            }
        );
    }
} // namespace codkit::lua
