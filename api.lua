module "global"

--- Gets a list of currently connected players.
--
-- @return list({ Name }}
function GetPlayers() end

--- Create a new timer.
--
-- @tparam function callback called when timer expires
-- @tparam number duration duration in milliseconds
-- @return number
function CreateTimer(callback, duration) end

--- Request team scores from the server.
--
--
function RequestScores() end

--- Execute a slash command.
--
-- @tparam string command command to execute
function exec(command) end

--- HttpServer class.
-- @type HttpServer

--- Create a new HttpServer object.
--
-- @return a HttpServer object
function HttpServer.new() end

--- Listen
--
-- @tparam number port port
function HttpServer:Listen(port) end

--- Listen
--
-- @tparam number port port
-- @tparam string host host
function HttpServer:Listen(port, host) end

--- Events.
-- The following functions will be executed if they have been defined.
-- @section events

--- Executed whenever a team's score has changed.
--
-- @tparam string team either "Axis" or "Allies"
-- @tparam number score the new score
OnScoreUpdated = function(team, score)  end
