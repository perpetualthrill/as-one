# As One Project

## Console

The console software sends and receives data from all of the other pieces. It is responsible for determining a good signal, updating the scoreboard, triggering the flame effect when queued by the operator.

### Backend development

The server code is written in Kotlin and located in the [console/backend/](./console/backend/) directory. To compile it, type './gradlew shadowJar' in that directory, which will assemble a fat jar containing all dependencies. The current static version of the frontend code is also included. To run that jar, do `java -jar build/libs/AsOne-Console-abc123-0.1-all.jar`, substituting the git hash from the actual filename for abc123. If this is successful, you will be able to access the running server at [http://localhost:12345/](http://localhost:12345/).

A JSON-encoded ordered list of the last 100 sensor readings can be read at via GET request to `/sensors/latest`

To test that and other services, fire up [Postman](https://www.getpostman.com/) and load [the AsOne-Console collection](./console/AsOne-Console.postman_collection.json). When adding or changing services, please update this collection, as it is effectively our API documentation.

### Sensor simulator

With the server running, simulated sensors can be used to test other parts of the server stack, such as scoreboard output. To create one of these, POST a request to `http://localhost:12345/sensors/simulated/add`. That will return a relative location fragment containing the simulator's name, which further actions can be directed to. To remove the simulator, send a DELETE request to the relevant sensor url. To simulate picking up the controller, reading a heartbeat, etc, please see the Postman collection linked above.

### Frontend development

The server's frontend is written in React using create-react-app, along with several UI libraries. The code may be found at [console/frontend/](./console/frontend/).

To get up and running with hacking on it, cd into that directory and issue `npm install`, presuming you have a working node + npm setup. When that is complete, you should be able to bring up the frontend in a browser with `npm run start`. The development server is set to proxy unknown request from port 3000, which it serves on, to port 12345, which is where the server is actually running. Thus, you won't have to wait for a server redeploy when making frontend changes.

The codebase conforms to [JavaScript Standard Style](https://standardjs.com/). To auto-format current changes to match, issue `npm run standardize`.

When shipping a feature, don't forget to do `npm run install`, which generates and copies the static files over into the server's static directory. We're currently checking them into source control there to avoid a more complicated build system -- c'est la vie.

## Sensor hardware

The heartrate sensor runs on an ESP-32 with the Arduino environment and reports its data over USB serial. The code may be found in [sketchbook-arduino/handheld-2/](./sketchbook-arduino/handheld-2/).


