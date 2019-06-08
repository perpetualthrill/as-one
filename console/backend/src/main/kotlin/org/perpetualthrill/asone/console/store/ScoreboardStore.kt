package org.perpetualthrill.asone.console.store

import org.perpetualthrill.asone.console.io.mqtt.MqttBroker
import org.perpetualthrill.asone.console.util.logInfo
import java.time.Instant
import javax.inject.Inject
import javax.inject.Singleton

private const val SCOREBOARD_DISCONNECT_THRESHOLD_MS = 2500

@Singleton
class ScoreboardStore
@Inject
constructor(mqtt: MqttBroker) {

    private val listener: MqttBroker.MqttListener

    private var lastHeartbeat: Instant? = null

    val connected: Boolean
        get() {
            val last = lastHeartbeat ?: return false
            if ((Instant.now().toEpochMilli() - last.toEpochMilli()) > SCOREBOARD_DISCONNECT_THRESHOLD_MS) return false
            return true
        }

    init {
        listener = object : MqttBroker.MqttListener() {
            override val topic = "asOne/score/heartbeat"
            override val handler = { content: String ->
                logInfo("Got a heartbeat: $content")
                lastHeartbeat = Instant.now()
            }
        }
        mqtt.registerListener(listener)
    }

}