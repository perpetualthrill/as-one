package org.perpetualthrill.asone.console.model

import io.reactivex.Observable
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.time.Instant
import java.util.*
import java.util.concurrent.TimeUnit


private const val TIME_TO_DISCONNECT_MS = 1500L
private const val RETAIN_PERIOD_MS = 5000L
private const val PRUNE_READINGS_INTERVAL_MS = 500L

class Sensor(val name: String) {

    private var lastReading = Instant.now()

    init {
        Observable
            .interval(PRUNE_READINGS_INTERVAL_MS, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                val retainPeriodEnd = Instant.now().minusMillis(RETAIN_PERIOD_MS)
                readings.removeIf {
                    it.timestamp.isBefore(retainPeriodEnd)
                }
            }
    }

    val readings = ArrayDeque<Reading>()

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
    }

    override fun toString(): String {
        return "Sensor $name"
    }

}