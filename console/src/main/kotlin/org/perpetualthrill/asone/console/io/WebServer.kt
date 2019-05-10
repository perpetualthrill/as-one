package org.perpetualthrill.asone.console.io

import io.ktor.application.call
import io.ktor.http.ContentType
import io.ktor.response.respondText
import io.ktor.routing.get
import io.ktor.routing.routing
import io.ktor.server.engine.embeddedServer
import io.ktor.server.netty.Netty
import org.slf4j.LoggerFactory
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class WebServer
@Inject
constructor() {

    val logger = LoggerFactory.getLogger(this.javaClass)

    fun start() {
        val server = embeddedServer(Netty, 12345) {
            routing {
                get("/") {
                    call.respondText("Hello, world!", ContentType.Text.Plain)
                }
            }
        }
        server.start()
        logger.info("as one console web server started")
    }

}