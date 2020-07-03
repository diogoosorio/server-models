require 'sinatra'


get '/' do
  #sleep(1)

  Array.new(1_000_000) { rand(100_000) }.sort!

  "Hello World\n"
end
