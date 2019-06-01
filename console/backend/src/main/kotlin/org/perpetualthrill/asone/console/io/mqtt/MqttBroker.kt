package org.perpetualthrill.asone.console.io.mqtt

import io.moquette.broker.Server
import java.util.Collections.singletonList
import io.moquette.broker.config.ClasspathResourceLoader;
import io.moquette.broker.config.ResourceLoaderConfig;
import io.moquette.interception.AbstractInterceptHandler
import io.moquette.interception.messages.InterceptPublishMessage
import org.perpetualthrill.asone.console.util.logInfo
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class MqttBroker
@Inject
constructor() {

    internal class PublisherListener : AbstractInterceptHandler() {
        override fun getID(): String {
            return "TestPublishListener"
        }

        override fun onPublish(msg: InterceptPublishMessage) {
            logInfo("Received on topic: " + msg.topicName + " content: " + msg.payload)
        }
    }

    fun start() {
        val classpathLoader = ClasspathResourceLoader()
        val classPathConfig = ResourceLoaderConfig(classpathLoader, "moquette.conf")

        val mqttBroker = Server()
        val userHandlers = singletonList(PublisherListener())
        mqttBroker.startServer(classPathConfig, userHandlers)
    }

}