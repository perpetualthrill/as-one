package org.perpetualthrill.asone.console.io

import com.github.sylvek.embbededmosquitto.Mosquitto
import org.eclipse.paho.client.mqttv3.*
import org.eclipse.paho.client.mqttv3.MqttConnectOptions.MQTT_VERSION_3_1_1
import org.perpetualthrill.asone.console.util.logInfo
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class MqttManager
@Inject
constructor() {

    private val listeners = mutableMapOf<String, List<MqttListener>>()

    private var client: IMqttAsyncClient? = null

    private val asyncActionListener = object : IMqttActionListener {
        override fun onSuccess(asyncActionToken: IMqttToken) {
            logInfo("mqtt success: $asyncActionToken")
            for (topic in listeners.keys) {
                client?.subscribe(topic, 0)
            }
        }

        override fun onFailure(asyncActionToken: IMqttToken, exception: Throwable) {
            logInfo("mqtt failure: ${asyncActionToken.exception}")
        }
    }

    private val mqttMessageCallback = object : MqttCallback {
        override fun messageArrived(topic: String, msg: MqttMessage) {
            val list = listeners[topic]
            list?.map { it.handler(msg.payload) }
        }

        override fun connectionLost(cause: Throwable) {
            logInfo("connection lost: ${cause.stackTrace}")
        }

        override fun deliveryComplete(token: IMqttDeliveryToken?) {
            // no-op
        }
    }

    fun start(enableInternalBroker: Boolean = true) {
        if (enableInternalBroker) {
            logInfo("Starting internal MQTT broker")
            Mosquitto.getInstance().start()
        }

        client = MqttAsyncClient("tcp://localhost:1883", "console")
        val options = MqttConnectOptions().apply {
            isCleanSession = true // do not send this client old stuff
            keepAliveInterval = 15 // keepalive call time for quiet connection in seconds
            connectionTimeout = 30 // time to wait for connection to appear in seconds
            isAutomaticReconnect = true // try to reconnect if connection lost. but uh, chances are we're fucked
            mqttVersion = MQTT_VERSION_3_1_1
        }
        client?.setCallback(mqttMessageCallback)
        client?.connect(options, asyncActionListener)
    }

    fun registerListener(listener: MqttListener) {
        val existing = listeners[listener.topic]
        val list = if (null != existing) {
            existing + listener
        } else {
            if (client?.isConnected == true) {
                client?.subscribe(listener.topic, 0)
            }
            listOf(listener)
        }
        listeners[listener.topic] = list
    }

    abstract class MqttListener {
        abstract val topic: String
        abstract val handler: (ByteArray) -> (Unit)
    }

    fun publishAtMostOnce(topic: String, byteArray: ByteArray) {
        // you know? i am not even going to mess with making an enum for QoS
        client?.publish(topic, byteArray, 0, false)
    }

}