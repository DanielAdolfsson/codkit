//
// Created by daniel on 10/10/23.
//

#include "httpserver.h"
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
            void(const std::shared_ptr<const HttpRequest> &, std::shared_ptr<HttpResponse> &)>
            &handler
    ) {
        httplib::Server::Get(
            pattern,
            [=](const httplib::Request &request, httplib::Response &response) {
                HttpContext context{
                    .request = request,
                    .response = response,
                    .handler = handler};
                SendMessage(
                    game::g_console_window,
                    WM_USER + 1,
                    (WPARAM)&context,
                    0
                );
            }
        );
    }

    HttpRequest::HttpRequest(const httplib::Request *request)
        : request(request) {
    }

    void HttpRequest::invalidate() {
        request = nullptr;
    }

    const std::string &HttpRequest::get_body() const {
        return request != nullptr ? request->body : empty_string;
    }

    HttpResponse::HttpResponse(httplib::Response *response)
        : response(response) {
    }

    void HttpResponse::invalidate() {
        response = nullptr;
    }

    const std::string &HttpResponse::get_body() const {
        return response != nullptr ? response->body : empty_string;
    }

    void HttpResponse::set_body(const std::string &value) {
        if (response != nullptr)
            response->body = value;
    }
} // namespace codkit::http
