package org.perpetualthrill.asone.console.state

import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.Color
import org.perpetualthrill.asone.console.model.Screen
import org.perpetualthrill.asone.console.model.ScreenConstants.SCREEN_HEIGHT
import org.perpetualthrill.asone.console.model.ScreenConstants.SCREEN_WIDTH
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.time.Instant
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

private const val SCOREBOARD_DISCONNECT_THRESHOLD_MS = 2500

@Singleton
class ScoreboardState
@Inject
constructor(private val mqtt: MqttManager, private val gameState: GameState) {

    private val heartbeatListener: MqttManager.MqttListener

    private var lastHeartbeat: Instant? = null

    val connected: Boolean
        get() {
            val last = lastHeartbeat ?: return false
            return (Instant.now().toEpochMilli() - last.toEpochMilli()) <= SCOREBOARD_DISCONNECT_THRESHOLD_MS
        }

    init {
        heartbeatListener = object : MqttManager.MqttListener() {
            override val topic = "asOne/score/heartbeat"
            override val handler = { _: ByteArray ->
                lastHeartbeat = Instant.now()
            }
        }
        mqtt.registerListener(heartbeatListener)
    }

    private val frameClock = Observable
        .interval(250, TimeUnit.MILLISECONDS)

    private fun makeColorBackground(frameNumber: Long): Screen {
        var counter = frameNumber * 25
        val newScreen = mutableListOf<Array<Color>>()
        for (i in 0..SCREEN_WIDTH) {
            val newColumn = mutableListOf<Color>()
            for (j in 0..SCREEN_HEIGHT) {
                counter += 25
                val color = Color(
                    counter.rem(256).toUByte(),
                    (counter + 100).rem(256).toUByte(),
                    (counter + 200).rem(256).toUByte()
                )
                newColumn.add(color)
            }
            newScreen.add(newColumn.toTypedArray())
        }
        return Screen(newScreen.toTypedArray())
    }

    private fun andBackgroundWithScore(frameNumber: Long): Screen {
        val background = makeColorBackground(frameNumber)
        // todo: this should be a zip operation
        val bpms = gameState.scores.take(1).blockingFirst()
        println(Screen.toTestString(Screen.renderBPMCharacters(bpms.left.toString())))
        return background
    }

    fun coloriffic() {
        frameClock.subscribeWithErrorLogging(this) { frameNumber ->
            val frame = andBackgroundWithScore(frameNumber)
            mqtt.publishAtMostOnce("asOne/score/all/direct", frame.toAsOneScoreboard.toByteArray())
        }
    }

}
