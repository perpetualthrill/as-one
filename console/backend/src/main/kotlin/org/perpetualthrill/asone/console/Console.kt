@file:JvmName("ConsoleStatic") // Necessary for shadowJar to find this as main
@file:JvmMultifileClass // Compile static main method outside class and merge to facade

package org.perpetualthrill.asone.console

import io.reactivex.Single
import org.perpetualthrill.asone.console.di.DaggerMainComponent
import org.perpetualthrill.asone.console.di.MainComponent
import org.perpetualthrill.asone.console.io.SerialMonitor
import org.perpetualthrill.asone.console.io.WebServer
import org.perpetualthrill.asone.console.io.mqtt.MqttBroker
import org.perpetualthrill.asone.console.store.ReadingStore
import org.perpetualthrill.asone.console.store.ScoreboardStore
import org.perpetualthrill.asone.console.util.logInfo
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.concurrent.TimeUnit
import javax.inject.Inject


class Console {

    @Inject lateinit var serialMonitor: SerialMonitor
    @Inject lateinit var readingStore: ReadingStore
    @Inject lateinit var webServer: WebServer
    @Inject lateinit var mqttBroker: MqttBroker
    @Inject lateinit var scoreboardStore: ScoreboardStore

    val mainComponent: MainComponent by lazy {
        DaggerMainComponent.builder().build()
    }

    init {
        INSTANCE = this
        mainComponent.inject(this)
    }

    fun actualMain() {
        serialMonitor.start()
        webServer.start()
        readingStore.start()
        mqttBroker.start()

        Single
            .just(true)
            .delay(3000, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                logInfo("Scoreboard connection status: " + scoreboardStore.connected)
                scoreboardStore.coloriffic()
            }

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
