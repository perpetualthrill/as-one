package org.perpetualthrill.asone.console.di

import dagger.Component
import org.perpetualthrill.asone.console.Console
import org.perpetualthrill.asone.console.model.SensorSimulator
import javax.inject.Singleton

@Singleton
@Component(modules = [MainModule::class])
interface MainComponent {

    fun inject(console: Console)

    // prefer autoinject by return here to calling inject from
    // objects themselves
    fun sensorSimulator(): SensorSimulator

}