package org.perpetualthrill.asone.console.model

import io.reactivex.Observable
import org.perpetualthrill.asone.console.io.SensorDataSource
import java.util.concurrent.TimeUnit
import javax.inject.Inject

class SensorSimulator
@Inject
constructor() {

    @Inject lateinit var sensorDataSource: SensorDataSource

    private val quiescentResults: SensorDataSource.ResultIterator by lazy {
        sensorDataSource.getQuiescentResults()
    }

    private var name = "unstarted!"
    private var state = SimulatorState.QUIESCENT
    private var finished = false

    // must lazy initialize so that the lateinit vars are filled
    // in by the time this is used. i.e. not at simulator init time
    private val resultIteratorForState by lazy { mapOf(
        SimulatorState.QUIESCENT to quiescentResults
    )}

    val readingStream: Observable<Sensor.Reading> = Observable
        .interval(20, TimeUnit.MILLISECONDS)
        .takeWhile { !finished }
        .map { checkStateAndReturnReading() }


    fun start(sensorName: String) {
        name = sensorName
    }

    private fun checkedGetIteratorForState(state: SimulatorState): SensorDataSource.ResultIterator {
        return resultIteratorForState[state] ?: throw RuntimeException("Oh dear, the SensorSimulator state machine broke!")
    }

    fun checkStateAndReturnReading(): Sensor.Reading {
        val currentStateIterator = checkedGetIteratorForState(state)
        val next = if (currentStateIterator.hasNext) {
            currentStateIterator.getNext()
        } else {
            state = state.nextState
            val newStateIterator = checkedGetIteratorForState(state)
            newStateIterator.reset()
            newStateIterator.getNext()
        }
        return Sensor.Reading(name, next.s1, next.s2, next.s3, next.s4)
    }

    fun finish() {
        finished = true
        sensorDataSource.finish()
    }

    private enum class SimulatorState {

        QUIESCENT {
            override val nextState = QUIESCENT
        };

        abstract val nextState: SimulatorState
    }

}