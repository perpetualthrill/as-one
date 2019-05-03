@file:JvmName("ConsoleStatic") // Necessary for shadowJar to find this as main
@file:JvmMultifileClass // Compile static main method outside class and merge to facade

package org.perpetualthrill.asone.console

import org.perpetualthrill.asone.console.di.DaggerMainComponent
import org.perpetualthrill.asone.console.di.MainComponent
import org.perpetualthrill.asone.console.io.SerialMonitor
import javax.inject.Inject

class Console {

    @Inject lateinit var serialMonitor: SerialMonitor

    private val mainComponent: MainComponent by lazy {
        DaggerMainComponent.builder().build()
    }

    init {
        mainComponent.inject(this)
    }

    fun actualMain() {
        println("Hello, world!")
        serialMonitor.hello()
    }

}

fun main() {
    Console().actualMain()
}
