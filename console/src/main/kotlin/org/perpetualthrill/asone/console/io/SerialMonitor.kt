package org.perpetualthrill.asone.console.io

import com.fazecast.jSerialComm.SerialPort
import com.fazecast.jSerialComm.SerialPortEvent
import com.fazecast.jSerialComm.SerialPortMessageListener
import io.reactivex.Flowable
import io.reactivex.Observable
import io.reactivex.processors.MulticastProcessor
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.*
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

private const val SENSOR_RETRY_MS = 1000L
private const val SENSOR_READING_BUFFER_SIZE = 100

@Singleton
class SerialMonitor
@Inject
constructor() {

    private data class PortAndSensor(
        val port: SerialPort,
        val sensor: Sensor
    )

    private val sensorAddresses = listOf("/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2")
    private val sensors = mutableMapOf<String, PortAndSensor>()

    private val sensorStreamInternal = MulticastProcessor.create<Sensor.Reading>()
    val sensorStream: Flowable<Sensor.Reading> = sensorStreamInternal

    private val sensorReadingBuffer = ArrayDeque<Sensor.Reading>(SENSOR_READING_BUFFER_SIZE)

    init {
        // necessary because it subscribes to nothing
        sensorStreamInternal.start()

        // maintain queue of last n readings for web service
        sensorStream.subscribeWithErrorLogging(this) {
            // data structure is not thread safe, but so long as this subscriber
            // is the only thing mutating it we are good
            sensorReadingBuffer.addFirst(it)
            while (sensorReadingBuffer.size > SENSOR_READING_BUFFER_SIZE) {
                sensorReadingBuffer.removeLast()
            }
        }
    }

    private inner class MessageListener(portName: String) : SerialPortMessageListener {

        val sensor = Sensor(portName)

        override fun delimiterIndicatesEndOfMessage(): Boolean {
            return true
        }

        override fun getMessageDelimiter(): ByteArray {
            return byteArrayOf(0x0A.toByte()) // carriage return aka '\n'
        }

        override fun getListeningEvents(): Int {
            return SerialPort.LISTENING_EVENT_DATA_RECEIVED
        }

        override fun serialEvent(event: SerialPortEvent) {
            val message = String(event.receivedData)
            try {
                val reading = sensor.readingFromSerialInput(message)
                sensorStreamInternal.offer(reading) // offer() will discard if nothing is listening
            } catch (_: Exception) { /* garbage inputs filtered here */ }
        }
    }

    fun start() {
        attemptConnectAll()
        Observable.interval(SENSOR_RETRY_MS, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                attemptConnectAll()
            }
    }

    private fun attemptConnectAll() {
        for (address in sensorAddresses) {
            // First, check for disconnects and remove
            val check = sensors[address]
            if (null != check && check.sensor.isDisconnected()) {
                sensors.remove(address)
                check.port.removeDataListener()
                check.port.closePort()
                println("SENSORS: "+sensors.keys)
            }
            // Now, attempt connect for any addresses not in map
            if(!sensors.containsKey(address)) {
                val portAndSensor = attemptConnect(address)
                if (null != portAndSensor) {
                    sensors[address] = portAndSensor
                    println("SENSORS: "+sensors.keys)
                }
            }
        }
    }

    private fun attemptConnect(address: String): PortAndSensor? {
        try {
            val port = SerialPort.getCommPort(address)
            port?.let {
                it.baudRate = 115200
                if(it.openPort(100)) {
                    val listener = MessageListener(address)
                    it.addDataListener(listener)
                    return PortAndSensor(it, listener.sensor)
                }
            }
        } catch (_: Exception) { }
        return null
    }

    val latestReadings
        get() = sensorReadingBuffer.toArray()

}