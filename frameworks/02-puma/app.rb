require 'sinatra'

hits = 0

get '/' do
  hits = hits + 1

  sleep(1)

  "PID: #{Process.pid} | hits: #{hits}\n"
end
