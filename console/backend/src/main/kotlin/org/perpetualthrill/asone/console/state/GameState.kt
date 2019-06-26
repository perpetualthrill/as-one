package org.perpetualthrill.asone.console.state

import io.reactivex.Observable
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class GameState
@Inject
constructor(sensorState: SensorState) {

    private var leftBPM: Int = 129
    private var rightBPM: Int = 138

    data class CurrentBPMs(
        val left: Int,
        val right: Int
    )

    init {
        sensorState.readingStream
            .subscribeWithErrorLogging(this) {

            }
    }

    val scores = Observable.create<CurrentBPMs> { emitter ->
        leftBPM++
        if (leftBPM > 199) leftBPM = 0
        rightBPM--
        if (rightBPM < 0) rightBPM = 199
        emitter.onNext(CurrentBPMs(leftBPM, rightBPM))
    }

}