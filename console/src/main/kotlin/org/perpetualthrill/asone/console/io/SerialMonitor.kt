package org.perpetualthrill.asone.console.io

import com.fazecast.jSerialComm.SerialPort
import com.fazecast.jSerialComm.SerialPortEvent
import com.fazecast.jSerialComm.SerialPortMessageListener
import java.io.IOException
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class SerialMonitor
@Inject
constructor() {

    var port0: SerialPort? = null
    var port1: SerialPort? = null

    private inner class MessageListener(val portName: String) : SerialPortMessageListener {
        override fun delimiterIndicatesEndOfMessage(): Boolean {
            return true;
        }

        override fun getMessageDelimiter(): ByteArray {
            return byteArrayOf(0x0A.toByte())
        }

        override fun getListeningEvents(): Int {
            return SerialPort.LISTENING_EVENT_DATA_RECEIVED;
        }

        override fun serialEvent(event: SerialPortEvent) {
            val message = String(event.receivedData)
            print("$portName received the following delimited message: $message")
        }
    }


    fun hello() {
        println("Hello SerialMonitor")
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
        println("$port0 -- $port1")
    }
}