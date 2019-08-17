package org.perpetualthrill.asone.console.io

import com.fazecast.jSerialComm.SerialPort
import com.fazecast.jSerialComm.SerialPortEvent
import com.fazecast.jSerialComm.SerialPortMessageListener
import io.reactivex.Observable
import io.reactivex.subjects.PublishSubject
import org.perpetualthrill.asone.console.model.Sensor
import org.perpetualthrill.asone.console.util.subscribeWithErrorLogging
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton


private const val SENSOR_RETRY_MS = 1000L

@Singleton
class SerialMonitor
@Inject
constructor() {

    private data class PortAndSensor(
        val port: SerialPort,
        val sensor: Sensor
    )

    private val sensorAddresses = listOf("/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2")
    private val userSensorAddresses = mutableListOf<String>()
    private val confirmedSensorMap = mutableMapOf<String, PortAndSensor>()
    private val sensorMap = mutableMapOf<String, PortAndSensor>()
    val sensors: List<Sensor>
        get() = confirmedSensorMap.values.map { it.sensor }

    private val sensorStreamInternal = PublishSubject.create<Sensor.Reading>()
    val sensorStream: Observable<Sensor.Reading> = sensorStreamInternal

    private inner class MessageListener(portName: String) : SerialPortMessageListener {

        val sensor = Sensor(portName.substring(1).replace('/', '_'))

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
                sensorStreamInternal.onNext(reading)
            } catch (_: Exception) { /* garbage inputs filtered here */ }
        }
    }

    fun start() {
        attemptConnectAll()
        Observable
            .interval(SENSOR_RETRY_MS, TimeUnit.MILLISECONDS)
            .subscribeWithErrorLogging(this) {
                attemptConnectAll()
            }
    }

    fun addSensorAddress(address: String) {
        userSensorAddresses.add(address)
    }

    private fun attemptConnectAll() {
        val allSensorAddresses = sensorAddresses + userSensorAddresses
        for (address in allSensorAddresses) {
            // First, check for disconnects and remove
            val check = sensorMap[address]
            if (null != check && check.sensor.isDisconnected()) {
                sensorMap.remove(address)
                confirmedSensorMap.remove(address)
                check.port.removeDataListener()
                check.port.closePort()
                check.sensor.finish()
            }
            // Now, attempt connect for any addresses not in map
            if(!sensorMap.containsKey(address)) {
                val portAndSensor = attemptConnect(address)
                if (null != portAndSensor) {
                    sensorMap[address] = portAndSensor
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
                    val gotOne = PortAndSensor(it, listener.sensor)
                    confirmedSensorMap[address] = gotOne
                    return gotOne
                }
            }
        } catch (_: Exception) { }
        return null
    }

}