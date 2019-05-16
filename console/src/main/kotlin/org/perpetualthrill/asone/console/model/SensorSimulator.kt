package org.perpetualthrill.asone.console.model

import org.perpetualthrill.asone.console.io.SensorCSVReader
import javax.inject.Inject

class SensorSimulator
@Inject
constructor() {

    @Inject lateinit var sensorCSVReader: SensorCSVReader

    fun start() {
        val resultIterator = sensorCSVReader.getQuiescentResults()
        for (i in 1..3) println("CHECK IT OUT: "+resultIterator.getNext())
    }

    fun finish() {
        sensorCSVReader.finish()
    }

}