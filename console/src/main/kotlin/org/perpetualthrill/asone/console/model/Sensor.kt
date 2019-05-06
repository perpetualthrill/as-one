package org.perpetualthrill.asone.console.model

class Sensor(private val name: String) {

    @Throws(RuntimeException::class) // parser errors
    fun readingFromSerialInput(input: String): Reading {
        val tokens = input.split(",")
        return Reading(
            sensorName = name,
            s1 = tokens[0].toInt(),
            s2 = tokens[1].toInt(),
            s3 = tokens[2].toInt(),
            s4 = tokens[3].toInt()
        )
    }

    data class Reading(
        val sensorName: String,
        val s1: Int,
        val s2: Int,
        val s3: Int,
        val s4: Int
    ) {
        override fun toString(): String {
            return "Sensor Reading, $sensorName: $s1 $s2 $s3 $s4"
        }
    }

    override fun toString(): String {
        return "Sensor $name"
    }

}