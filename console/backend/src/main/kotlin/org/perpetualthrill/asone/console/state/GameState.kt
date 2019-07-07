package org.perpetualthrill.asone.console.state

import com.kizitonwose.time.milliseconds
import com.kizitonwose.time.minutes
import com.kizitonwose.time.plus
import com.kizitonwose.time.seconds
import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.SENSOR_UPDATE_INTERVAL
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.util.logError
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

private val BPM_UPDATE_INTERVAL = 500.milliseconds

private const val MAX_BPM = 133
private val MAX_BPM_INTERVAL = (60.seconds / MAX_BPM).inMilliseconds
private const val MIN_BPM = 60
private val MIN_BPM_INTERVAL = (60.seconds / MIN_BPM).inMilliseconds

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

    // Keep knowledge of which sensor is in which position
    val gameSensors = mutableMapOf<String, SensorPosition>()

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

    // magic numbers! any reading outside this range is pretty much guaranteed to be bogus
    private fun readingIsGlitch(reading: Int): Boolean {
        return (reading < 200 || reading > 800)
    }

    private fun updateBPMs() {
        for (sensorName in gameSensors.keys) {
            // get readings. go to next if funny
            val readings = try {
                sensorState.readingsForSensor(sensorName)
            } catch (e: Exception) {
                // probably the sensor was removed but is still in our array
                logError("error getting readings: $e")
                continue
            }
            if (readings.size < 10) continue

            // pivot readings into int arrays. accumulate glitch list
            val pivotReadings = listOf(IntArray(readings.size), IntArray(readings.size), IntArray(readings.size), IntArray(readings.size))
            val glitches = listOf(BooleanArray(readings.size), BooleanArray(readings.size), BooleanArray(readings.size), BooleanArray(readings.size))
            for (sensorIndex in 0..3) {
                for (readingIndex in readings.indices) {
                    val reading = readings[readingIndex].asArray[sensorIndex]
                    pivotReadings[sensorIndex][readingIndex] = reading
                    glitches[sensorIndex][readingIndex] = readingIsGlitch(reading)
                }
            }

            // find IBI candidates
            val ibiCandidatesMS = mutableListOf<Long>()
            for (sensorIndex in 0..3) {
                // calculate the lower bound of the upper quartile. if there are no good ones,
                // continue to the next
                val sorted = pivotReadings[sensorIndex].filter { !readingIsGlitch(it) }.sorted()
                if (sorted.size < 10) continue // sometimes this is zero and crashes. here is a sane magic number limit
                val upperQuartile = sorted[(sorted.size * 3) / 4]

                // walk the readings array looking for candidate beats, i.e. several top
                // quartile readings in a row
                val beatCandidates = BooleanArray(readings.size)
                for (readingIndex in 0..(readings.size - 3)) {
                    if (!glitches[sensorIndex][readingIndex]
                        && pivotReadings[sensorIndex][readingIndex] >= upperQuartile
                        && pivotReadings[sensorIndex][readingIndex+1] >= upperQuartile
                        && pivotReadings[sensorIndex][readingIndex+2] >= upperQuartile) beatCandidates[readingIndex] = true
                }

                // now, walk the candidates array and accumulate
                var foundFirst = false
                var accumulator = 0.milliseconds
                for (beatIndex in beatCandidates.indices) {
                    if (beatCandidates[beatIndex]) {
                        // notably, this avoids the case of multiple candidates in a row, because those
                        // will necessarily have accumulator == 0
                        if (foundFirst && accumulator > MAX_BPM_INTERVAL && accumulator < MIN_BPM_INTERVAL) {
                            ibiCandidatesMS.add(accumulator.longValue)
                        }

                        // zero accumulator so next non-candidate can accumulate into it
                        accumulator = 0.milliseconds

                        // set flag for valid accumulations henceforth
                        if (!foundFirst) foundFirst = true
                    } else {
                        // for non-beats, add their interval to the ibi accumulator
                        accumulator += SENSOR_UPDATE_INTERVAL
                    }
                }
            }

            // sort IBIs and determine BPM. skip if low IBI count -- guarantees
            // the sensor is just seeing noise
            if (ibiCandidatesMS.size < 8) continue
            ibiCandidatesMS.sort()
            val bpm = 1.minutes.inMilliseconds / ibiCandidatesMS[ibiCandidatesMS.size / 2]
            when (gameSensors[sensorName]) {
                SensorPosition.LEFT -> leftBPM = bpm.longValue.toInt()
                SensorPosition.RIGHT -> rightBPM = bpm.longValue.toInt()
            }

        }

        lastBPMUpdate = Calendar.getInstance()
    }

}
