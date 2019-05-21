package org.perpetualthrill.asone.console.store

import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject
import org.perpetualthrill.asone.console.di.Injector
import org.perpetualthrill.asone.console.io.SerialMonitor
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.model.SensorSimulator
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

private const val SENSOR_READING_BUFFER_SIZE = 100

@Singleton
class ReadingStore
@Inject
constructor(
    private val serialMonitor: SerialMonitor
) {
    private val internalReadingStream = PublishSubject.create<Sensor.Reading>()
    val readingStream: Observable<Sensor.Reading> = internalReadingStream

    private val readingBuffer = ArrayDeque<Sensor.Reading>(SENSOR_READING_BUFFER_SIZE)

    private val simulators = mutableMapOf<String, SensorSimulator>()

    fun start() {
        serialMonitor.sensorStream.subscribeWith(internalReadingStream)

        // maintain queue of last n readings for web service
        readingStream.subscribeWithErrorLogging(this) {
            // data structure is not thread safe, but so long as this subscriber
            // is the only thing mutating it we are good
            readingBuffer.addFirst(it)
            while (readingBuffer.size > SENSOR_READING_BUFFER_SIZE) {
                readingBuffer.removeLast()
            }
        }
    }

    val latestReadings: Array<out Any>
        get() = readingBuffer.toArray()

    fun addSimulator(): String {
        val simulator = Injector.get().sensorSimulator()
        val name = "simulator${simulator.hashCode()}"
        simulator.start(name)
        simulators[name] = simulator
        simulator.readingStream.subscribeWith(internalReadingStream)
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

    fun updateSimulatorState(name: String, newStateString: String): Boolean {
        val simulator = simulators[name]
        if (null != simulator) {
            val newState = SensorSimulator.SimulatorState.forString(newStateString)
            simulator.updateState(newState)
            return true
        }
        return false
    }

}