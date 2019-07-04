package org.perpetualthrill.asone.console.model

import com.kizitonwose.time.milliseconds
import io.reactivex.Observable
import io.reactivex.disposables.Disposable
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.time.Instant
import java.util.*
import java.util.concurrent.TimeUnit

private const val TIME_TO_DISCONNECT_MS = 1500L
private const val RETAIN_PERIOD_MS = 5000L
private const val PRUNE_READINGS_INTERVAL_MS = 500L

// Ideally this constant would be shared centrally within the project.
// Current firmware updates at 50hz, i.e. a 20ms interval
val SENSOR_UPDATE_INTERVAL = 20.milliseconds

class Sensor(val name: String) {

    private var finished = false

    private var lastReading = Instant.now()

    private val pruneTask: Disposable

    init {
        pruneTask = Observable
            .interval(PRUNE_READINGS_INTERVAL_MS, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                val retainPeriodEnd = Instant.now().minusMillis(RETAIN_PERIOD_MS)
                readings.removeIf {
                    it.timestamp.isBefore(retainPeriodEnd)
                }
            }
    }

    val readings = ArrayDeque<Reading>()
        get() {
            if (finished) throw RuntimeException("Sensor read after finished!")
            return field
        }

    @Throws(RuntimeException::class) // parser errors
    fun readingFromSerialInput(input: String): Reading {
        val tokens = input.split(",")
        val timestamp = Instant.now()
        lastReading = timestamp
        val reading = Reading(
            sensorName = name,
            s1 = tokens[0].toInt(),
            s2 = tokens[1].toInt(),
            s3 = tokens[2].toInt(),
            s4 = tokens[3].toInt(),
            timestamp = timestamp
        )
        readings.addFirst(reading)
        return reading
    }

    fun isDisconnected(): Boolean {
        return Instant.now().isAfter(lastReading.plusMillis(TIME_TO_DISCONNECT_MS))
    }

    fun finish() {
        readings.clear()
        pruneTask.dispose()
        finished = true
    }

    data class Reading(
        val sensorName: String,
        val s1: Int,
        val s2: Int,
        val s3: Int,
        val s4: Int,
        val timestamp: Instant = Instant.now()
    ) {
        override fun toString(): String {
            return "Sensor reading for $sensorName: $s1 $s2 $s3 $s4 at $timestamp"
        }

        fun toCSVString(): String {
            return "$sensorName,$s1,$s2,$s3,$s4,${timestamp.toEpochMilli()}"
        }
    }

    override fun toString(): String {
        return "Sensor $name"
    }

}