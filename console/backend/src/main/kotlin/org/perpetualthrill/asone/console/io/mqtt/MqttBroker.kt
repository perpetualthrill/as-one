package org.perpetualthrill.asone.console.io.mqtt

import com.github.sylvek.embbededmosquitto.Mosquitto
import org.fusesource.mqtt.client.FutureConnection
import org.fusesource.mqtt.client.MQTT
import org.fusesource.mqtt.client.QoS
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class MqttBroker
@Inject
constructor() {

    private val listeners = mutableMapOf<String, List<MqttListener>>()

    private val mqttClient = MQTT()
    private var mqttClientConnection: FutureConnection? = null

    fun start() {
        Mosquitto.getInstance().start()

        mqttClient.setHost("localhost", 1883)
        mqttClient.setClientId("console")
        mqttClient.version = "3.1.1"
        mqttClient.isCleanSession = true // retain nothing
        mqttClient.keepAlive = 15 // send keepalive message after 15s idle
        mqttClientConnection = mqttClient.futureConnection()
        mqttClientConnection?.connect()
    }

    fun registerListener(listener: MqttListener) {
        val existing = listeners[listener.topic]
        val list = if (null != existing) {
            existing + listener
        } else {
            listOf(listener)
        }
        listeners[listener.topic] = list
    }

    abstract class MqttListener {
        abstract val topic: String
        abstract val handler: (String) -> (Unit)
    }

    fun publishAtMostOnce(topic: String, byteArray: ByteArray) {
        mqttClientConnection?.publish(topic, byteArray, QoS.AT_MOST_ONCE, false)
    }

}