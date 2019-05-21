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
import org.perpetualthrill.asone.console.store.ReadingStore
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import org.slf4j.LoggerFactory
import java.util.concurrent.TimeUnit
import javax.inject.Inject


class Console {

    @Inject lateinit var serialMonitor: SerialMonitor
    @Inject lateinit var readingStore: ReadingStore
    @Inject lateinit var webServer: WebServer

    val mainComponent: MainComponent by lazy {
        DaggerMainComponent.builder().build()
    }

    init {
        INSTANCE = this
        mainComponent.inject(this)
    }

    fun actualMain() {
        val loggerContext = LoggerFactory.getILoggerFactory() as LoggerContext
        val rootLogger = loggerContext.getLogger(Logger.ROOT_LOGGER_NAME)
        rootLogger.level = Level.INFO

        serialMonitor.start()
        webServer.start()
        readingStore.start()

        readingStore.readingStream
            .sample(1, TimeUnit.SECONDS)
            .subscribeWithErrorLogging(this) {
                println("$it")
            }
    }

    companion object {
        // kinda weird way to do this, but makes it possible to get the
        // dagger injector from anywhere. if something manages to call this
        // before it is initialized, it may be a misunderstanding on the
        // caller's part -jc
        private lateinit var INSTANCE: Console
        fun getInstance(): Console {
            return INSTANCE
        }
    }

}

fun main() {
    Console().actualMain()
}
