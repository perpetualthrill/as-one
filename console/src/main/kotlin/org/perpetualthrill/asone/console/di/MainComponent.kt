package org.perpetualthrill.asone.console.di

import dagger.Component
import org.perpetualthrill.asone.console.Console
import javax.inject.Singleton

@Singleton
@Component(modules = [MainModule::class])
interface MainComponent {
    fun inject(console: Console)
}