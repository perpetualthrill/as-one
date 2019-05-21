package org.perpetualthrill.asone.console.model

import io.reactivex.Observable
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

    private var name = "unstarted!"
    private var currentState = SimulatorState.QUIESCENT
    private var finished = false

    val readingStream: Observable<Sensor.Reading> = Observable
        .interval(20, TimeUnit.MILLISECONDS)
        .takeWhile { !finished }
        .map { checkStateAndReturnReading() }

    private val allResults = mutableMapOf<SimulatorState, Results>()

    fun start(sensorName: String) {
        name = sensorName
    }

    fun updateState(newState: SimulatorState) {
        logger.debug("Updating simulator $name from $currentState to $newState ")
        currentState = newState
    }

    private fun checkStateAndReturnReading(): Sensor.Reading {
        val currentStateIterator = resultsForState(currentState)
        val next = if (currentStateIterator.hasNext) {
            // while we have results, use one
            currentStateIterator.advance()
            currentStateIterator.get()
        } else {
            // if we're out of results:
            // 1) advance to the next state
            // 2) reset the reader to start, as it is in an unknown state
            // 3) return the first row
            updateState(currentState.nextState)
            val nextStateIterator = resultsForState(currentState)
            nextStateIterator.reset()
            nextStateIterator.get()
        }
        return Sensor.Reading(name, next.s1, next.s2, next.s3, next.s4)
    }

    fun finish() {
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

    enum class SimulatorState {

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

        fun get(): Sensor.Reading {
            return sensorReadingFromCurrentRow()
        }

        fun reset() {
            resultSet.first()
        }

        private fun sensorReadingFromCurrentRow(): Sensor.Reading {
            return Sensor.Reading(
                sensorName = name,
                s1 = resultSet.getInt(1),
                s2 = resultSet.getInt(2),
                s3 = resultSet.getInt(3),
                s4 = resultSet.getInt(4)
            )
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
            return v
        }
    }
}