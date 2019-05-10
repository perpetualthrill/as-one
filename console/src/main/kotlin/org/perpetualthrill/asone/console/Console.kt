@file:JvmName("ConsoleStatic") // Necessary for shadowJar to find this as main
@file:JvmMultifileClass // Compile static main method outside class and merge to facade

package org.perpetualthrill.asone.console

import ch.qos.logback.classic.Level
import ch.qos.logback.classic.Logger
import ch.qos.logback.classic.LoggerContext
import org.perpetualthrill.asone.console.di.DaggerMainComponent
import org.perpetualthrill.asone.console.di.MainComponent
import org.perpetualthrill.asone.console.io.SerialMonitor
import org.perpetualthrill.asone.console.io.WebServer
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import org.slf4j.LoggerFactory
import javax.inject.Inject


class Console {

    @Inject lateinit var serialMonitor: SerialMonitor
    @Inject lateinit var webServer: WebServer

    private val mainComponent: MainComponent by lazy {
        DaggerMainComponent.builder().build()
    }

    init {
        mainComponent.inject(this)
    }

    fun actualMain() {
        val loggerContext = LoggerFactory.getILoggerFactory() as LoggerContext
        val rootLogger = loggerContext.getLogger(Logger.ROOT_LOGGER_NAME)
        rootLogger.level = Level.INFO

        serialMonitor.init()
        serialMonitor.sensorStream.subscribeWithErrorLogging(this) {
            println("$it")
        }
        webServer.start()
    }

}

fun main() {
    Console().actualMain()
}
