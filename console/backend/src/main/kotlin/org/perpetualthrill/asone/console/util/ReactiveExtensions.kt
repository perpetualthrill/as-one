package org.perpetualthrill.asone.console.util

import io.reactivex.Flowable
import io.reactivex.Observable
import io.reactivex.Single
import io.reactivex.disposables.Disposable
import org.slf4j.LoggerFactory

private fun makeErrorHandler(tag: String): (Throwable) -> Unit = { e: Throwable ->
    val logger = LoggerFactory.getLogger(tag)
    logger.error("Subscription error!", e)
}

fun <T : Any> Flowable<T>.subscribeWithErrorLogging(
    self: Any = this,
    onError: (Throwable) -> Unit = makeErrorHandler(self.javaClass.canonicalName),
    onComplete: () -> Unit = { },
    onNext: (T) -> Unit = { }
): Disposable = subscribe(onNext, onError, onComplete)

fun <T : Any> Observable<T>.subscribeWithErrorLogging(
    self: Any = this,
    onError: (Throwable) -> Unit = makeErrorHandler(self.javaClass.canonicalName),
    onComplete: () -> Unit = { },
    onNext: (T) -> Unit = { }
): Disposable = subscribe(onNext, onError, onComplete)

fun <T : Any> Single<T>.subscribeWithErrorLogging(
    self: Any = this,
    onError: (Throwable) -> Unit = makeErrorHandler(self.javaClass.canonicalName),
    onSuccess: (T) -> Unit = { }
): Disposable = subscribe(onSuccess, onError)