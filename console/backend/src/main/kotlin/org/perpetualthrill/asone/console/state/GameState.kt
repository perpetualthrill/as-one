package org.perpetualthrill.asone.console.state

import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class GameState
@Inject
constructor(
    private val sensorState: SensorState,
    mqttManager: MqttManager
) {

    // Bogus start values. Frozen on these indicates an error
    private var leftBPM: Int = 199
    private var rightBPM: Int = 188

    // Keep internal knowledge of which sensor is in which position
    private val gameSensors = mutableMapOf<String, SensorPosition>()

    enum class SensorPosition {
        LEFT, RIGHT
    }

    data class CurrentBPMs(
        val left: Int,
        val right: Int
    )

    init {
        sensorState.readingStream
            .subscribeWithErrorLogging(this) { reading ->
                // change state based on reading
                val position = processSensorReading(reading)
                // forward to mqtt if valid
                if (null != position) {
                    val csvReading = reading.toCSVString()
                    // add position column
                    mqttManager.publishAtMostOnce("asOne/sensor/reading", "$csvReading,${position.name}".toByteArray())
                }
            }
    }

    private fun processSensorReading(sensorReading: Sensor.Reading): SensorPosition? {
        // first, check if we're already monitoring this sensor
        val current = gameSensors[sensorReading.sensorName]
        if (null != current) return current

        // next, if there are any dead sensors, clear them so we can hang on
        // to this new one
        val active = sensorState.activeSensorNames
        for (name in gameSensors.keys) {
            if (!active.contains(name)) {
                gameSensors.remove(name)
            }
        }

        // finally, check and see if there's a place for the new sensor and
        // store + return it if so
        val newPosition = when (gameSensors.size) {
            2 -> return null // nope, all full
            1 -> if (gameSensors.values.contains(SensorPosition.RIGHT)) SensorPosition.LEFT else SensorPosition.RIGHT // replace!
            0 -> SensorPosition.LEFT // first in goes to left
            else -> throw RuntimeException("Too many sensors bro") // Uh-oh, crashy crashy
        }
        gameSensors[sensorReading.sensorName] = newPosition
        return newPosition
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