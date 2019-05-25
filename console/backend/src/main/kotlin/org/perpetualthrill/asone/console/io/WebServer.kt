package org.perpetualthrill.asone.console.io

import io.ktor.application.call
import io.ktor.application.install
import io.ktor.features.Compression
import io.ktor.features.ContentNegotiation
import io.ktor.gson.gson
import io.ktor.http.HttpStatusCode
import io.ktor.http.content.defaultResource
import io.ktor.http.content.resources
import io.ktor.http.content.static
import io.ktor.locations.*
import io.ktor.request.receive
import io.ktor.response.respond
import io.ktor.response.respondText
import io.ktor.routing.get
import io.ktor.routing.post
import io.ktor.routing.route
import io.ktor.routing.routing
import io.ktor.server.engine.embeddedServer
import io.ktor.server.netty.Netty
import org.perpetualthrill.asone.console.store.ReadingStore
import org.slf4j.Logger
import org.slf4j.LoggerFactory
import javax.inject.Inject
import javax.inject.Singleton

@KtorExperimentalLocationsAPI
@Singleton
class WebServer
@Inject
constructor(
    private val readingStore: ReadingStore
) {

    private val logger: Logger = LoggerFactory.getLogger(this.javaClass)

    @Location("/sensors/simulated/{name}")
    data class Simulator(
        val name: String
    )

    fun start() {
        val server = embeddedServer(Netty, 12345) {
            install(Compression)
            install(ContentNegotiation) {
                gson {
                    setPrettyPrinting()
                    serializeNulls()
                }
            }
            install(Locations)

            routing {
                // Serve web page and related content
                static("/") {
                    resources("web-static")
                    defaultResource("web-static/index.html")
                }

                route("/sensors") {
                    get("latest") {
                        call.respond(readingStore.latestReadings)
                    }
                    route("simulated") {
                        post("add") {
                            val name = readingStore.addSimulator()
                            val href = locations.href(Simulator(name=name))
                            call.respondText(text = href, status = HttpStatusCode.Created)
                        }
                    }
                }

                get<Simulator> { simulator ->
                    call.respondText("Simulator at ${locations.href(simulator)}")
                }
                delete<Simulator> { simulator ->
                    if (readingStore.removeSimulator(simulator.name)) {
                        call.respondText("OK")
                    } else {
                        call.respondText("Not found: ${simulator.name}", status = HttpStatusCode.NotFound)
                    }
                }
                patch<Simulator> { simulator ->
                    val update = call.receive<SimulatorUpdate>()
                    if (readingStore.updateSimulatorState(simulator.name, update.newState)) {
                        call.respondText("OK")
                    } else {
                        call.respondText("Not found: ${simulator.name}", status = HttpStatusCode.NotFound)
                    }
                }


            }
        }
        server.start()
        logger.info("as one console web server started")
    }

}

data class SimulatorUpdate(
    val newState: String
)