@file:JvmName("ConsoleStatic") // Necessary for shadowJar to find this as main
@file:JvmMultifileClass // Compile static main method outside class and merge to facade

package org.perpetualthrill.asone.console

import com.github.ajalt.clikt.core.CliktCommand
import com.github.ajalt.clikt.parameters.options.default
import com.github.ajalt.clikt.parameters.options.flag
import com.github.ajalt.clikt.parameters.options.option
import io.reactivex.Single
import org.perpetualthrill.asone.console.di.DaggerMainComponent
import org.perpetualthrill.asone.console.di.MainComponent
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.io.SerialMonitor
import org.perpetualthrill.asone.console.io.WebServer
import org.perpetualthrill.asone.console.state.ScoreboardState
import org.perpetualthrill.asone.console.state.SensorState
import org.perpetualthrill.asone.console.util.logInfo
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.concurrent.TimeUnit
import javax.inject.Inject


class Console : CliktCommand() {

    @Inject lateinit var serialMonitor: SerialMonitor
    @Inject lateinit var sensorState: SensorState
    @Inject lateinit var webServer: WebServer
    @Inject lateinit var mqttManager: MqttManager
    @Inject lateinit var scoreboardState: ScoreboardState

    // clikt args
    private val disableSerialArgument by option("--disable-serial", help = "turn off USB serial monitoring").flag()
    private val hostArgument: String by option("--hostname", help = "hostname or ip address to bind services to. defaults to 192.168.12.1").default("192.168.12.1")
    private val usbArgument: String by option("--usb", help = "additional usb filesystem location to check for a handset").default("")

    val mainComponent: MainComponent by lazy {
        DaggerMainComponent.builder().build()
    }

    init {
        INSTANCE = this
        mainComponent.inject(this)
    }

    override fun run() {
        if (!disableSerialArgument) {
            serialMonitor.start()
            if (!usbArgument.isNullOrBlank()) {
                serialMonitor.addSensorAddress(usbArgument)
            }
        }
        webServer.start(hostName = hostArgument)
        sensorState.start()
        mqttManager.start(hostName = hostArgument)

        Single
            .just(true)
            .delay(3000, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                logInfo("Scoreboard connection status: " + scoreboardState.connected)
                scoreboardState.coloriffic()
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

fun main(args: Array<String>) {
    Console().main(args)
}
