package org.perpetualthrill.asone.console.io

import io.ktor.application.call
import io.ktor.application.install
import io.ktor.features.Compression
import io.ktor.features.ContentNegotiation
import io.ktor.gson.gson
import io.ktor.http.ContentType
import io.ktor.response.respond
import io.ktor.response.respondText
import io.ktor.routing.get
import io.ktor.routing.routing
import io.ktor.server.engine.embeddedServer
import io.ktor.server.netty.Netty
import org.slf4j.Logger
import org.slf4j.LoggerFactory
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class WebServer
@Inject
constructor(
    private val serialMonitor: SerialMonitor
) {

    private val logger: Logger = LoggerFactory.getLogger(this.javaClass)

    fun start() {
        val server = embeddedServer(Netty, 12345) {
            install(Compression)
            install(ContentNegotiation) {
                gson {
                    setPrettyPrinting()
                    serializeNulls()
                }
            }

            routing {
                get("/") {
                    call.respondText("Hello, world!", ContentType.Text.Plain)
                }

                get("/sensors/latest") {
                    call.respond(serialMonitor.latestReadings)
                }
            }
        }
        server.start()
        logger.info("as one console web server started")
    }

}