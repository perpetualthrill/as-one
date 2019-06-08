package org.perpetualthrill.asone.console.io.mqtt

import io.moquette.broker.Server
import java.util.Collections.singletonList
import io.moquette.broker.config.ClasspathResourceLoader
import io.moquette.broker.config.ResourceLoaderConfig
import io.moquette.interception.AbstractInterceptHandler
import io.moquette.interception.messages.InterceptPublishMessage
import org.perpetualthrill.asone.console.util.logInfo
import java.nio.charset.Charset
import java.nio.charset.StandardCharsets
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class MqttBroker
@Inject
constructor() {

    private val listeners = mutableMapOf<String, List<MqttListener>>()

    inner class PublisherListener : AbstractInterceptHandler() {
        override fun getID(): String {
            return "EverythingListener"
        }

        override fun onPublish(msg: InterceptPublishMessage) {
            val list = listeners[msg.topicName]
            if (!list.isNullOrEmpty()) {
                val content = msg.payload.toString(StandardCharsets.UTF_8)
                for (listener in list) {
                    listener.handler(content)
                }
            }
        }
    }

    fun start() {
        val classpathLoader = ClasspathResourceLoader()
        val classPathConfig = ResourceLoaderConfig(classpathLoader, "moquette.conf")

        val mqttBroker = Server()
        val userHandlers = singletonList(PublisherListener())
        mqttBroker.startServer(classPathConfig, userHandlers)
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

}