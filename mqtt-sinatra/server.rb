require 'sinatra'
require 'mqtt'

get '/' do
  publish("asOne/hello","hello from server!")
  "Hello World"
end

get '/testUpper/:duration' do
  doPoof("Upper", params[:duration])
end

get '/testLower/:duration' do
  doPoof("Lower", params[:duration])
end

def doPoof(output, durationStr)
  duration = durationStr.to_i
  if (duration < 100)
    duration = 100
  elsif (duration > 999)
    duration = 999
  end
  publish("asOne/test#{output}", duration)
  "poofing #{output}: #{duration}"
end

def publish(topic, message)
  MQTT::Client.connect('asone-console') do |c|
    c.publish(topic, message)
  end
end

