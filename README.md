# As One Project

## Server software

The server code written in Kotlin and located in the [console/backend/](./console/backend/) directory. To compile it, type './gradlew shadowJar' in that directory to assemble a fat jar containing all dependencies. To run that jar, do `java -jar build/libs/AsOne-Console-0.1-all.jar` -- if this is successful, you will be able to access the running server at [http://localhost:12345/](http://localhost:12345/).

A JSON-encoded ordered list of the last 100 sensor readings can be read at via GET request to `/sensors/latest`

To test that and other services, fire up [Postman](https://www.getpostman.com/) and load [the AsOne-Console collection](./console/AsOne-Console.postman_collection.json). When adding or changing services, please update this collection, as it is effectively our API documentation.

### Sensor simulator

With the server running, simulated sensors can be used to test other parts of the server stack, such as scoreboard output. To create one of these, POST a request to `http://localhost:12345/sensors/simulated/add`. That will return a relative location fragment containing the simulator's name, which further actions can be directed to. To remove the simulator, send a DELETE request to the relevant sensor url. To simulate picking up the controller, reading a heartbeat, etc, please see the Postman collection linked above.

## Sensor hardware

The heartrate sensor runs on an ESP-32 with the Arduino environment and reports its data over USB serial. The code may be found in [sketchbook-arduino/handheld-2/](./sketchbook-arduino/handheld-2/).


