package org.perpetualthrill.asone.console.io

import com.fazecast.jSerialComm.SerialPort
import com.fazecast.jSerialComm.SerialPortEvent
import com.fazecast.jSerialComm.SerialPortMessageListener
import io.reactivex.Flowable
import io.reactivex.processors.MulticastProcessor
import org.perpetualthrill.asone.console.model.Sensor
import java.io.IOException
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class SerialMonitor
@Inject
constructor() {

    private val sensorStreamInternal = MulticastProcessor.create<Sensor.Reading>()
    val sensorStream: Flowable<Sensor.Reading> = sensorStreamInternal

    private var port0: SerialPort? = null
    private var port1: SerialPort? = null

    init {
        sensorStreamInternal.start() // necessary because it subscribes to nothing
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

    // TODO: Retry this all the time for plug / unplug support
    fun init() {
        try {
            port0 = SerialPort.getCommPort("/dev/ttyUSB0")
            port0?.baudRate = 115200
            port0?.openPort()
            port0?.addDataListener(MessageListener("usb0"))
        } catch (_: IOException) { }
        try {
            port1 = SerialPort.getCommPort("/dev/ttyUSB1")
            port1?.baudRate = 115200
            port1?.openPort()
            port1?.addDataListener(MessageListener("usb1"))
        } catch (_: IOException) { }
    }

}