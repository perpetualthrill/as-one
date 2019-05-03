package org.perpetualthrill.asone.console.io

import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class SerialMonitor
@Inject
constructor() {
    fun hello() {
        println("Hello SerialMonitor")
    }
}