package org.perpetualthrill.asone.console.store

import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.util.logDebug
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.time.Instant
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

private const val SCOREBOARD_DISCONNECT_THRESHOLD_MS = 2500

@Singleton
class ScoreboardStore
@Inject
constructor(private val mqtt: MqttManager) {

    private val listener: MqttManager.MqttListener

    private var lastHeartbeat: Instant? = null

    val connected: Boolean
        get() {
            val last = lastHeartbeat ?: return false
            if ((Instant.now().toEpochMilli() - last.toEpochMilli()) > SCOREBOARD_DISCONNECT_THRESHOLD_MS) return false
            return true
        }

    init {
        listener = object : MqttManager.MqttListener() {
            override val topic = "asOne/score/heartbeat"
            override val handler = { content: ByteArray ->
                logDebug("Got a heartbeat: ${content.toString(Charsets.UTF_8)}")
                lastHeartbeat = Instant.now()
            }
        }
        mqtt.registerListener(listener)
    }

    private val frameClock = Observable
        .interval(250, TimeUnit.MILLISECONDS)

    private fun makeCRGBArray(pixelLength: Int, counterStart: Long): ByteArray {
        var counter = counterStart
        val array = ByteArray(pixelLength * 3)
        for (i in array.indices step 3) {
            counter += 25
            array[i] = counter.rem(256).toByte()
            array[i + 1] = (counter + 100).rem(256).toByte()
            array[i + 2] = (counter + 200).rem(256).toByte()
        }
        return array
    }

    fun coloriffic() {
        mqtt.publishAtMostOnce("asOne/scoreboard/directOnly", byteArrayOf(1))
        mqtt.publishAtMostOnce("asOne/scoreboard/acceleration", byteArrayOf(4))
        frameClock.subscribeWithErrorLogging(this) {
            val counterStart = it * 25 // start each frame a bit further
            val byteArray = makeCRGBArray(135, counterStart) // full scoreboard is 135 pixels
            mqtt.publishAtMostOnce("asOne/score/all/direct", byteArray)
        }
    }

}
