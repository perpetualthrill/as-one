package org.perpetualthrill.asone.console.state

import io.reactivex.Observable
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class GameState
@Inject
constructor(sensorState: SensorState) {

    // Bogus start values. Frozen on these indicates an error
    private var leftBPM: Int = 199
    private var rightBPM: Int = 188

    data class CurrentBPMs(
        val left: Int,
        val right: Int
    )

    init {
        sensorState.readingStream
            .subscribeWithErrorLogging(this) {

            }
    }

    // Remove this when actual sensor code exists
    var frameCount = 0

    val bpms = Observable.create<CurrentBPMs> { emitter ->
        frameCount++
        // Once per second update the scores
        if (frameCount % SCOREBOARD_UPDATE_FPS == 0) {
            leftBPM++
            if (leftBPM > 199) leftBPM = 0
            rightBPM--
            if (rightBPM < 0) rightBPM = 199
        }
        emitter.onNext(CurrentBPMs(leftBPM, rightBPM))
    }

}