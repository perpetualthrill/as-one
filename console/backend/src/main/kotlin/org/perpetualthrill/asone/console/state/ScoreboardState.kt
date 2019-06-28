package org.perpetualthrill.asone.console.state

import com.kizitonwose.time.microseconds
import com.kizitonwose.time.plus
import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.Color
import org.perpetualthrill.asone.console.model.Screen
import org.perpetualthrill.asone.console.model.ScreenConstants.LEFT_BPM_START_X
import org.perpetualthrill.asone.console.model.ScreenConstants.LEFT_BPM_START_Y
import org.perpetualthrill.asone.console.model.ScreenConstants.RIGHT_BPM_START_X
import org.perpetualthrill.asone.console.model.ScreenConstants.RIGHT_BPM_START_Y
import org.perpetualthrill.asone.console.model.ScreenConstants.SCREEN_HEIGHT
import org.perpetualthrill.asone.console.model.ScreenConstants.SCREEN_WIDTH
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

        // generate rainbowey background
        var counter = slowerFrame * 25
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

    // Use utility function to bitwise AND the scoreboard characters onto
    // whatever the current background is
    private fun andBackgroundWithBPM(frameNumber: Long, bpms: GameState.CurrentBPMs): Screen {
        val background = makeCurrentBackground(frameNumber)
        val leftBPMArray = Screen.renderBPMCharacters(bpms.left.toString())
        background.andWithBytes(leftBPMArray, LEFT_BPM_START_X, LEFT_BPM_START_Y)
        val rightBPMArray = Screen.renderBPMCharacters(bpms.right.toString())
        background.andWithBytes(rightBPMArray, RIGHT_BPM_START_X, RIGHT_BPM_START_Y)
        return background
    }

    // temporary start function for testing
    fun coloriffic() {
        frameClock.subscribeWithErrorLogging(this) { frameNumber ->
            val currentBPMs = gameState.bpms.take(1).blockingFirst()
            val frame = andBackgroundWithBPM(frameNumber, currentBPMs)
            mqtt.publishAtMostOnce("asOne/score/all/direct", frame.toAsOneScoreboard().toByteArray())
        }
    }

}
