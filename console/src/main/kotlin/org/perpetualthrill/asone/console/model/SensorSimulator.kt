package org.perpetualthrill.asone.console.model

import org.perpetualthrill.asone.console.io.SensorCSVReader
import javax.inject.Inject

class SensorSimulator
@Inject
constructor() {

    @Inject lateinit var sensorCSVReader: SensorCSVReader

    fun test() {
        val foo = sensorCSVReader
        println("CHECK IT $foo")
    }

}