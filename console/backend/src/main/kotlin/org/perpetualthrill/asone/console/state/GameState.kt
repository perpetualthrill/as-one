package org.perpetualthrill.asone.console.state

import com.kizitonwose.time.milliseconds
import com.kizitonwose.time.minutes
import com.kizitonwose.time.plus
import com.kizitonwose.time.seconds
import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.MqttManager
import org.perpetualthrill.asone.console.model.SENSOR_UPDATE_INTERVAL
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.util.CircularArray
import org.perpetualthrill.asone.console.util.logError
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

private val BPM_UPDATE_INTERVAL = 500.milliseconds

private const val MAX_BPM = 125
private val MAX_BPM_INTERVAL = (60.seconds / MAX_BPM).inMilliseconds
private const val MIN_BPM = 60
private val MIN_BPM_INTERVAL = (60.seconds / MIN_BPM).inMilliseconds

private const val BPM_BUFFER_SIZE = 10

@Singleton
class GameState
@Inject
constructor(
    private val sensorState: SensorState,
    private val mqttManager: MqttManager
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

    // Average across the last few BPM readings
    private val leftBuffer = CircularArray<Int>(BPM_BUFFER_SIZE)
    private val rightBuffer = CircularArray<Int>(BPM_BUFFER_SIZE)

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
        // ugh this is a getter with side effects :-0
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

    private fun getBPM(sensorName: String): Double? {
        // get readings. go to next if funny
        val readings = try {
            sensorState.readingsForSensor(sensorName)
        } catch (e: Exception) {
            // probably the sensor was removed but is still in our array
            logError("error getting readings: $e")
            return null
        }
        if (readings.size < 10) return null

        // pivot readings into int arrays. accumulate glitch list
        val pivotReadings = listOf(IntArray(readings.size), IntArray(readings.size), IntArray(readings.size), IntArray(readings.size))
        val glitches = listOf(BooleanArray(readings.size), BooleanArray(readings.size), BooleanArray(readings.size), BooleanArray(readings.size))
        for (sensorIndex in 0..3) {
            for (readingIndex in readings.indices) {
                // NOTE: there is a transient NPE here with the simulator data. which does not
                // seem possible because it loops and the readings manipulation above is
                // safety checked? but yeah, i dunno
                try {
                    val reading = readings[readingIndex].asArray[sensorIndex]
                    pivotReadings[sensorIndex][readingIndex] = reading
                    glitches[sensorIndex][readingIndex] = readingIsGlitch(reading)
                } catch (_: NullPointerException) {
                    return null
                }
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
            var inBeat = false
            for (readingIndex in 0..(readings.size - 3)) {
                if (!glitches[sensorIndex][readingIndex]
                    && pivotReadings[sensorIndex][readingIndex] >= upperQuartile
                    && pivotReadings[sensorIndex][readingIndex+1] >= upperQuartile
                    && pivotReadings[sensorIndex][readingIndex+2] >= upperQuartile) {
                    // only record a hit for the first of these -- use onset as a substitute
                    // for calculating IBI peak-to-peak
                    if (!inBeat) {
                        beatCandidates[readingIndex] = true
                        inBeat = true
                    }
                } else {
                    inBeat = false
                }
            }

            // now, walk the candidates array and accumulate
            var foundFirst = false
            var accumulator = 0.milliseconds
            for (beatIndex in beatCandidates.indices) {
                accumulator += SENSOR_UPDATE_INTERVAL
                if (beatCandidates[beatIndex]) {
                    // if accumulator is within valid range, save it as a candidate
                    if (foundFirst && accumulator > MAX_BPM_INTERVAL && accumulator < MIN_BPM_INTERVAL) {
                        ibiCandidatesMS.add(accumulator.longValue)
                    }

                    // zero accumulator so next non-candidate can accumulate into it. only do this
                    // if it has exceeded the minimum -- otherwise it's glitch and should be ignored
                    if (accumulator > MAX_BPM_INTERVAL) {
                        accumulator = 0.milliseconds
                    }

                    // set flag for valid accumulations henceforth. avoids garbage initial interval
                    if (!foundFirst) foundFirst = true
                }
            }
        }


        // skip if low IBI count -- guarantees the sensor is just seeing noise
        // this needs a little love -- what if one of the sensors is
        // disabled, or we otherwise don't have a full count? this should
        // be a state transition rather than a hard-coded number
        if (ibiCandidatesMS.size < 8) return null

        // sort candidates to determine median
        ibiCandidatesMS.sort()

        // cheat: whack the lowest two. something in this algo is running warm, and
        // this cools it down. bleh :-/
        ibiCandidatesMS.removeAt(0)
        ibiCandidatesMS.removeAt(0)

        /*
        // Logging stuff
        println("handset name $sensorName")

        val modeMap = mutableMapOf<Int, Int>()
        print("IBIs: ")
        for (candidate in ibiCandidatesMS) {
            val fred = (1.minutes.inMilliseconds.longValue.toDouble() / candidate.toDouble()).toInt()
            print("$fred ")
            val barney = modeMap[fred]
            if (null != barney) {
                modeMap[fred] = barney + 1
            } else {
                modeMap[fred] = 1
            }
        }
        var highestValue = 0
        var highestCount = 0
        for (pair in modeMap.entries) {
            if (pair.value > highestValue) {
                highestValue = pair.value
                highestCount = pair.key
            }
        }
        println()
        println("median: ${(1.minutes.inMilliseconds.longValue.toDouble()) / (ibiCandidatesMS[ibiCandidatesMS.size / 2].toDouble())} mode: $highestCount ")
        println()
        */

        return (1.minutes.inMilliseconds.longValue.toDouble()) / (ibiCandidatesMS[ibiCandidatesMS.size / 2].toDouble())
    }

    // average bpm readings, but if there are too many misses, return
    // a miss
    private fun averageBuffer(buffer: CircularArray<Int>): Int {
        if (buffer.size < BPM_BUFFER_SIZE) return -1
        var misses = 0
        var accumulator = 0
        for (value in buffer) {
            if (value == -1) {
                misses++
            } else {
                accumulator += value
            }
        }
        if (misses > (BPM_BUFFER_SIZE / 2)) return -1
        return (accumulator / (BPM_BUFFER_SIZE - misses))
    }

    private fun updateBPMs() {
        for (sensorName in gameSensors.keys) {
            val bpm = getBPM(sensorName)
            val fixBPM = bpm?.toInt() ?: -1
            when (gameSensors[sensorName]) {
                SensorPosition.LEFT -> {
                    leftBuffer.add(fixBPM)
                    leftBPM = averageBuffer(leftBuffer)
                }
                SensorPosition.RIGHT -> {
                    rightBuffer.add(fixBPM)
                    rightBPM = averageBuffer(rightBuffer)
                }
            }
        }

        lastBPMUpdate = Calendar.getInstance()
    }

}
