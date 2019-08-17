package org.perpetualthrill.asone.console.model

import io.reactivex.Observable
import io.reactivex.Observer
import io.reactivex.observers.DisposableObserver
import org.slf4j.Logger
import org.slf4j.LoggerFactory
import org.xbib.jdbc.io.TableReader
import java.io.InputStreamReader
import java.io.Reader
import java.sql.Connection
import java.sql.DriverManager
import java.sql.ResultSet
import java.sql.Statement
import java.util.*
import java.util.concurrent.TimeUnit
import javax.inject.Inject

class SensorSimulator
@Inject
constructor() {

    private val logger: Logger = LoggerFactory.getLogger(this.javaClass)

    private val connection: Connection

    init {
        connection = DriverManager.getConnection("jdbc:xbib:csv:class:"+ResourceFileReader::class.java.name)
    }

    private var currentState = SimulatorState.QUIESCENT
    private var finished = false
    private var nonComplete: NonComplete? = null

    lateinit var sensor: Sensor

    private val readingStream: Observable<Sensor.Reading> = Observable
        .interval(SENSOR_UPDATE_INTERVAL.longValue, TimeUnit.MILLISECONDS)
        .takeWhile { !finished }
        .map { checkStateAndReturnReading() }

    private val allResults = mutableMapOf<SimulatorState, Results>()

    fun start(sensorName: String) {
        sensor = Sensor(sensorName)
    }

    fun startSimple(sensorName: String, magicNumber: Int) {
        currentState = SimulatorState.SIMPLE
        allResults[SimulatorState.SIMPLE] = makeSimpleResults(magicNumber)
        start(sensorName)
    }

    fun updateState(newState: SimulatorState) {
        logger.debug("Updating simulator ${sensor.name} from $currentState to $newState ")
        currentState = newState
    }

    private class NonComplete(private val wrappedObserver: Observer<Sensor.Reading>) : DisposableObserver<Sensor.Reading>() {
        override fun onComplete() {
            // no-op. make completion go away
        }

        override fun onNext(t: Sensor.Reading) {
            wrappedObserver.onNext(t)
        }

        override fun onError(e: Throwable) {
            wrappedObserver.onError(e)
        }
    }

    private fun checkStateAndReturnReading(): Sensor.Reading {
        val currentStateIterator = resultsForState(currentState)
        val next = if (currentStateIterator.hasNext) {
            // while we have results, use one
            currentStateIterator.advance()
            currentStateIterator.getSensorOutputLine()
        } else {
            // if we're out of results:
            // 1) advance to the next state
            // 2) reset the reader to start, as it is in an unknown state
            // 3) return the first row
            updateState(currentState.nextState)
            val nextStateIterator = resultsForState(currentState)
            nextStateIterator.reset()
            nextStateIterator.getSensorOutputLine()
        }
        return sensor.readingFromSerialInput(next)
    }

    fun subscribeObserver(obs: Observer<Sensor.Reading>) {
        val nc = NonComplete(obs)
        readingStream.subscribe(nc)
        nonComplete = nc
    }

    fun finish() {
        nonComplete?.dispose()
        sensor.finish()
        finished = true
        connection.close() // also closes all resultsets
    }

    private fun resultsForState(state: SimulatorState): Results {
        val found = allResults[state]
        if (null != found) return found
        val made = makeResults(state)
        allResults[state] = made
        return made
    }

    private fun makeResults(state: SimulatorState): Results {
        val statement = connection.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_READ_ONLY)
        val resultSet = statement.executeQuery("select s1, s2, s3, s4, state from log-data-20190512 where state = '${state.stateColumnValue}'")
        return Results(resultSet)
    }

    private fun makeSimpleResults(magicNumber: Int): Results {
        val tableName = "ctrlr-$magicNumber-log-80bpm"
        val statement = connection.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_READ_ONLY)
        val resultSet = statement.executeQuery("select s1, s2, s3, s4, 'whatever' as state from $tableName")
        return Results(resultSet)
    }

    enum class SimulatorState {

        SIMPLE {
            override val nextState = SIMPLE
            override val stateColumnValue = "simple"
        },

        QUIESCENT {
            override val nextState = QUIESCENT
            override val stateColumnValue = "quiescent"
        },

        HEARTBEAT {
            override val nextState = HEARTBEAT
            override val stateColumnValue = "heartbeat"
        },

        PICKUP {
            override val nextState = HEARTBEAT
            override val stateColumnValue = "pickup"
        },

        SETDOWN {
            override val nextState = QUIESCENT
            override val stateColumnValue = "setdown"
        };

        abstract val nextState: SimulatorState
        abstract val stateColumnValue: String

        companion object {
            fun forString(value: String): SimulatorState {
                return when (value.toLowerCase()) {
                    QUIESCENT.name.toLowerCase() -> QUIESCENT
                    HEARTBEAT.name.toLowerCase() -> HEARTBEAT
                    PICKUP.name.toLowerCase() -> PICKUP
                    SETDOWN.name.toLowerCase() -> SETDOWN
                    SIMPLE.name.toLowerCase() -> SIMPLE
                    else -> throw RuntimeException("Yikes! Asked simulator for unknown state: $value")
                }
            }
        }
    }

    // Note! This class does a lot of real borderline shit. Check hasNext before calling getNext
    // on a one shot result set or it will throw
    inner class Results(private val resultSet: ResultSet) {

        val hasNext: Boolean
            get() = !resultSet.isAfterLast

        fun advance() {
            resultSet.next()
        }

        fun getSensorOutputLine(): String {
            return "${resultSet.getInt(1)},${resultSet.getInt(2)},${resultSet.getInt(3)},${resultSet.getInt(4)}, etc"
        }

        fun reset() {
            resultSet.first()
        }

    }

    class ResourceFileReader : TableReader {
        override fun getReader(statement: Statement, tableName: String): Reader {
            val inputStream = javaClass.getResourceAsStream("/$tableName.csv")
            return InputStreamReader(inputStream)
        }

        override fun getTableNames(connection: Connection): List<String> {
            val v = Vector<String>()
            v.add("log-data-20190512")
            v.add("ctrlr-1-log-80bpm")
            v.add("ctrlr-2-log-80bpm")
            return v
        }
    }
}