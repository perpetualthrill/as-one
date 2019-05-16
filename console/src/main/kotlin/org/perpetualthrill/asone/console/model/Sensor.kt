package org.perpetualthrill.asone.console.model

import java.time.Instant
import java.time.temporal.Temporal

private const val TIME_TO_DISCONNECT_MS = 1500L

class Sensor(private val name: String) {

    private var lastReading = Instant.now()

    @Throws(RuntimeException::class) // parser errors
    fun readingFromSerialInput(input: String): Reading {
        val tokens = input.split(",")
        val timestamp = Instant.now()
        lastReading = timestamp
        return Reading(
            sensorName = name,
            s1 = tokens[0].toInt(),
            s2 = tokens[1].toInt(),
            s3 = tokens[2].toInt(),
            s4 = tokens[3].toInt(),
            timestamp = timestamp
        )
    }

    fun isDisconnected(): Boolean {
        return (Instant.now().isAfter(lastReading.plusMillis(TIME_TO_DISCONNECT_MS)))
    }

    data class Reading(
        val sensorName: String,
        val s1: Int,
        val s2: Int,
        val s3: Int,
        val s4: Int,
        val timestamp: Temporal = Instant.now()
    ) {
        override fun toString(): String {
            return "Sensor reading for $sensorName: $s1 $s2 $s3 $s4 at $timestamp"
        }
    }

    override fun toString(): String {
        return "Sensor $name"
    }

}