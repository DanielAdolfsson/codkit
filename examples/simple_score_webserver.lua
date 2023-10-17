if server == nil then
    server = HttpServer.new()
    server:Listen(8888)
end

server:Mount("/", "./html")

server:Get("/status", function()
    return HttpServerResponse
            .new()
            :WithBody({
        Axis = Axis or 0,
        Allies = Allies or 0
    })
end)
