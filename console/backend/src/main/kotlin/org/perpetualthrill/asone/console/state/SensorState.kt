package org.perpetualthrill.asone.console.state

import com.kizitonwose.time.seconds
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject
import org.perpetualthrill.asone.console.di.Injector
import org.perpetualthrill.asone.console.io.SerialMonitor
import org.perpetualthrill.asone.console.model.SENSOR_UPDATE_INTERVAL
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.model.SensorSimulator
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

private val SENSOR_READING_BUFFER_TIME = 5.seconds
private val SENSOR_READING_SAMPLE_SIZE = (SENSOR_READING_BUFFER_TIME.longValue / SENSOR_UPDATE_INTERVAL.longValue).toInt()

@Singleton
class SensorState
@Inject
constructor(
    private val serialMonitor: SerialMonitor
) {
    private val internalReadingStream = PublishSubject.create<Sensor.Reading>()
    val readingStream: Observable<Sensor.Reading> = internalReadingStream

    private val readingBuffer = ArrayDeque<Sensor.Reading>(SENSOR_READING_SAMPLE_SIZE)

    private val simulators = mutableMapOf<String, SensorSimulator>()

    fun start() {
        serialMonitor.sensorStream.subscribeWith(internalReadingStream)

        // maintain queue of last n readings for web service
        readingStream.subscribeWithErrorLogging(this) { reading ->
            // data structure is not thread safe, but so long as this subscriber
            // is the only thing mutating it we are good
            readingBuffer.addFirst(reading)
            while (readingBuffer.size > SENSOR_READING_SAMPLE_SIZE) {
                // not sure why this occasionally throws, but it does
                try {
                    readingBuffer.removeLast()
                } catch (e: NoSuchElementException) { }
            }
        }
    }

    val latestReadings: Array<out Any>
        get() = readingBuffer.toArray()

    val activeSensorNames: List<String>
        get() = (simulators.keys + serialMonitor.sensors.map { it.name }).toList()

    private inline fun <reified T> fixArray(list: List<*>): Array<T> {
        return (list as List<T>).toTypedArray()
    }

    fun readingsForSensor(name: String): Array<Sensor.Reading> {

        simulators[name]?.let {
            return fixArray(it.sensor.readings.toArray().asList())
        }
        serialMonitor.sensors.firstOrNull { it.name == name }?.let {
            return fixArray(it.readings.toArray().asList())
        }
        throw Exception("Sensor not found: name")
    }

    fun addSimulator(): String {
        val simulator = Injector.get().sensorSimulator()
        val name = "simulator${simulator.hashCode()}"
        simulator.start(name)
        simulators[name] = simulator
        simulator.subscribeObserver(internalReadingStream)
        return name
    }

    fun removeSimulator(name: String): Boolean {
        val simulator = simulators[name]
        if (null != simulator) {
            simulators.remove(name)
            simulator.finish()
            return true
        }
        return false
    }

    fun updateSimulatorState(simulatorName: String, newStateString: String): Boolean {
        val simulator = simulators[simulatorName]
        if (null != simulator) {
            val newState = SensorSimulator.SimulatorState.forString(newStateString)
            simulator.updateState(newState)
            return true
        }
        return false
    }

}