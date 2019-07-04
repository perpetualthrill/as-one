package org.perpetualthrill.asone.console.state

import com.kizitonwose.time.milliseconds
import com.kizitonwose.time.plus
import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

private val BPM_UPDATE_INTERVAL = 500.milliseconds

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

    // Record start time and initialize other timing
    private val startTime = Calendar.getInstance()
    private var lastSensorReading = Calendar.getInstance()
    private var lastBPMUpdate = Calendar.getInstance()

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
                val position = updateSensorPositions(reading)
                // forward to mqtt if valid
                if (null != position) {
                    val csvReading = reading.toCSVString()
                    // add position column
                    mqttManager.publishAtMostOnce("asOne/sensor/reading", "$csvReading,${position.name}".toByteArray())
                }
                // bump sensor reading time regardless
                lastSensorReading = Calendar.getInstance()
            }
    }

    private fun updateSensorPositions(sensorReading: Sensor.Reading): SensorPosition? {
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

    val bpms = Observable.create<CurrentBPMs> { emitter ->
        // update these values from time to time
        if (lastSensorReading > (lastBPMUpdate + BPM_UPDATE_INTERVAL)) {
            updateBPMs()
        }

        // then return them
        emitter.onNext(CurrentBPMs(leftBPM, rightBPM))
    }

    // if there's one sensor, change its position. if there are two,
    // flip them. if there are zero or error state return
    fun flipSensors() {
        val ref = gameSensors
        val keys = ref.keys.toList()
        when (ref.size) {
            2 -> {
                // wow there is probably a less fugly way to do this!
                val oldZeroPosition = ref[keys[0]] as SensorPosition
                val oldOnePosition = ref[keys[1]] as SensorPosition
                gameSensors[keys[0]] = oldOnePosition
                gameSensors[keys[1]] = oldZeroPosition
            }
            1 -> gameSensors[keys[0]] = if (ref[keys[0]] == SensorPosition.LEFT) SensorPosition.RIGHT else SensorPosition.LEFT
            // else do nothing
        }
    }

    private fun updateBPMs() {
        lastBPMUpdate = Calendar.getInstance()
        leftBPM++
        if (leftBPM > 199) leftBPM = 0
        rightBPM--
        if (rightBPM < 0) rightBPM = 199
    }

}
