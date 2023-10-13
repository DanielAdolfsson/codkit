//
// Created by daniel on 10/10/23.
//

#include "httpserver.h"
#include "detours.h"
#include "game.h"

namespace codkit::http {
    static std::string empty_string;

    void HttpServer::listen(const std::string &host, int port) {
        std::thread{[this, host, port]() {
            httplib::Server::listen(host, port, 0);
        }}.detach();
    }

    void HttpServer::get(
        const std::string &pattern,
        const std::function<
            void(const HttpRequest &, const std::shared_ptr<HttpResponse> &)>
            &handler
    ) {
        httplib::Server::Get(
            pattern,
            [=](const httplib::Request &request, httplib::Response &response) {
                detours::run_on_main_thread([&]() {
                    HttpRequest httpRequest{.body = request.body};
                    auto httpResponse = std::make_shared<HttpResponse>();
                    handler(httpRequest, httpResponse);
                    response.body = httpResponse->body;
                });
            }
        );
    }
} // namespace codkit::http
