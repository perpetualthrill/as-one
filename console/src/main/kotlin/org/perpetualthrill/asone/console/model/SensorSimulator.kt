package org.perpetualthrill.asone.console.model

import io.reactivex.Observable
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

    private val connection: Connection

    init {
        connection = DriverManager.getConnection("jdbc:xbib:csv:class:"+ResourceFileReader::class.java.name)
    }

    private var name = "unstarted!"
    private var state = SimulatorState.QUIESCENT
    private var finished = false

    val readingStream: Observable<Sensor.Reading> = Observable
        .interval(20, TimeUnit.MILLISECONDS)
        .takeWhile { !finished }
        .map { checkStateAndReturnReading() }

    fun start(sensorName: String) {
        name = sensorName
    }

    private fun checkStateAndReturnReading(): Sensor.Reading {
        val currentStateIterator = resultIteratorOrThrow(state)
        val next = if (currentStateIterator.hasNext) {
            currentStateIterator.getNext()
        } else {
            state = state.nextState
            val nextStateIterator = resultIteratorOrThrow(state)
            nextStateIterator.reset()
            nextStateIterator.getNext()
        }
        return Sensor.Reading(name, next.s1, next.s2, next.s3, next.s4)
    }

    fun finish() {
        finished = true
        connection.close() // also closes all resultsets
    }

    private fun resultIteratorOrThrow(state: SimulatorState): ResultIterator {
        return allResults[state] ?: throw RuntimeException("Oh dear! The sensor simulator state machine broke")
    }

    private val quiescentResults: ResultIterator by lazy {
        // val countStmt = connection.createStatement(ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_READ_ONLY)
        val statement = connection.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_READ_ONLY)
        val resultSet = statement.executeQuery("select s1, s2, s3, s4, state from log-data-20190512 where state = 'quiescent'")
        ResultIterator(resultSet, false)
    }

    private val allResults: Map<SimulatorState, ResultIterator> by lazy {
        mapOf(SimulatorState.QUIESCENT to quiescentResults)
    }

    private enum class SimulatorState {

        QUIESCENT {
            override val nextState = QUIESCENT
            override val stateColumnValue = "quiescent"
        };

        abstract val nextState: SimulatorState
        abstract val stateColumnValue: String
    }

    // Note! This class does a lot of real borderline shit. Check hasNext before calling getNext
    // on a one shot result set or it will throw
    inner class ResultIterator(private val resultSet: ResultSet, private val oneShot: Boolean) {

        val hasNext = if (oneShot) !resultSet.isAfterLast else true

        fun getNext(): Sensor.Reading {
            val gotNext = resultSet.next()
            if (!gotNext && !oneShot) {
                reset()
            }
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