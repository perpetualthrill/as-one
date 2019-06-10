package org.perpetualthrill.asone.console.store

import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.mqtt.MqttBroker
import org.perpetualthrill.asone.console.util.logInfo
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.time.Instant
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

private const val SCOREBOARD_DISCONNECT_THRESHOLD_MS = 2500

@Singleton
class ScoreboardStore
@Inject
constructor(private val mqtt: MqttBroker) {

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

    fun coloriffic() {
        mqtt.publishAtMostOnce("asOne/scoreboard/directOnly", byteArrayOf(1))
        mqtt.publishAtMostOnce("asOne/scoreboard/acceleration", byteArrayOf(4))
        Observable
            .fromIterable(object : Iterable<ByteArray> {
                override fun iterator(): Iterator<ByteArray> {
                    return object : Iterator<ByteArray> {

                        val SIZE = 15 * 3

                        var counter: Long = 100

                        override fun hasNext(): Boolean {
                            return true
                        }

                        override fun next(): ByteArray {
                            val logo = ByteArray(SIZE)
                            for (i in logo.indices step 3) {
                                counter += 25
                                logo[i] = counter.rem(256).toByte()
                                logo[i + 1] = (counter + 100).rem(256).toByte()
                                logo[i + 2] = (counter + 200).rem(256).toByte()
                            }
                            return logo
                        }

                    }
                }
            })
            .sample(250, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                mqtt.publishAtMostOnce("asOne/score/logo/direct", it)
            }
    }

}
