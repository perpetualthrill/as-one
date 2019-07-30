package org.perpetualthrill.asone.console.state

import com.kizitonwose.time.microseconds
import com.kizitonwose.time.plus
import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.Color
import org.perpetualthrill.asone.console.model.Screen
import org.perpetualthrill.asone.console.model.ScreenConstants.leftBPMRect
import org.perpetualthrill.asone.console.model.ScreenConstants.rightBPMRect
import org.perpetualthrill.asone.console.model.ScreenConstants.screenRect
import org.perpetualthrill.asone.console.model.ScreenConstants.timerRect
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

private val SCOREBOARD_DISCONNECT_THRESHOLD = 2500.microseconds
const val SCOREBOARD_UPDATE_FPS = 24
private val SCOREBOARD_UPDATE_INTERVAL = 1000.microseconds / SCOREBOARD_UPDATE_FPS

@Singleton
class ScoreboardState
@Inject
constructor(private val mqtt: MqttManager, private val gameState: GameState) {

    private val heartbeatListener: MqttManager.MqttListener

    private var lastHeartbeat: Calendar? = null

    val connected: Boolean
        get() {
            val last = lastHeartbeat ?: return false
            return (Calendar.getInstance().compareTo(last + SCOREBOARD_DISCONNECT_THRESHOLD)) > 0
        }

    private val frameClock = Observable
        .interval(SCOREBOARD_UPDATE_INTERVAL.longValue, TimeUnit.MILLISECONDS)

    init {
        heartbeatListener = object : MqttManager.MqttListener() {
            override val topic = "asOne/score/heartbeat"
            override val handler = { _: ByteArray ->
                lastHeartbeat = Calendar.getInstance()
            }
        }
        mqtt.registerListener(heartbeatListener)
    }

    // in the future this will make several kinds of effect backgrounds
    // depending on gamestate
    private fun makeCurrentBackground(frameNumber: Long): Screen {
        // slow down this effect by only generating a new one every few frames
        val slowerFrame = frameNumber / 3

        // calculate a small offset so each frame changes slightly
        val offset = when (frameNumber.rem(3)) {
            2L -> 0
            1L -> 1
            0L -> 2
            else -> 0
        } * 5

        // generate rainbowey background
        var counter = slowerFrame * 25
        val newScreen = mutableListOf<Array<Color>>()
        for (i in 0..screenRect.width) {
            val newColumn = mutableListOf<Color>()
            for (j in 0..screenRect.height) {
                counter += 25
                val color = Color(
                    (counter.rem(256) - offset).toUByte(),
                    ((counter + 100).rem(256) - offset).toUByte(),
                    ((counter + 200).rem(256) - offset).toUByte()
                )
                newColumn.add(color)
            }
            newScreen.add(newColumn.toTypedArray())
        }
        return Screen(newScreen.toTypedArray())
    }

    private fun renderBPM(bpm: Int): Array<UByteArray> {
        val bpmString = if (bpm == -1) "___" else bpm.toString()
        return Screen.renderBPMCharacters(bpmString)
    }

    // Use utility function to bitwise AND the scoreboard characters onto
    // whatever the current background is
    private fun andBackgroundWithBPM(frameNumber: Long, bpms: GameState.CurrentBPMs): Screen {
        val background = makeCurrentBackground(frameNumber)

        // clear timer area
        background.fill(timerRect, Color.BLACK)

        // fill in digits
        val leftBPMArray = renderBPM(bpms.left)
        background.andWithBytes(leftBPMArray, leftBPMRect.location.x, leftBPMRect.location.y)
        val rightBPMArray = renderBPM(bpms.right)
        background.andWithBytes(rightBPMArray, rightBPMRect.location.x, rightBPMRect.location.y)

        return background
    }

    // temporary start function for testing
    fun coloriffic() {
        frameClock.subscribeWithErrorLogging(this) { frameNumber ->
            val currentBPMs = gameState.bpms.take(1).blockingFirst()
            val frame = andBackgroundWithBPM(frameNumber, currentBPMs)
            mqtt.publishAtMostOnce("asOne/score/all/direct", frame.toAsOneScoreboard().toByteArray())
            mqtt.publishAtMostOnce("asOne/console/leftBPM", "${currentBPMs.left}".toByteArray())
            mqtt.publishAtMostOnce("asOne/console/rightBPM", "${currentBPMs.right}".toByteArray())
        }
    }

}
