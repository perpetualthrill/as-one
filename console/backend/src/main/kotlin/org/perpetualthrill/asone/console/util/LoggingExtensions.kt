package org.perpetualthrill.asone.console.util

import org.slf4j.LoggerFactory

fun Any.logInfo(message: String) {
    val logger = LoggerFactory.getLogger(this.javaClass.name)
    logger.info(message)
}

fun Any.logDebug(message: String) {
    val logger = LoggerFactory.getLogger(this.javaClass.name)
    logger.debug(message)
}

fun Any.logError(message: String) {
    val logger = LoggerFactory.getLogger(this.javaClass.name)
    logger.error(message)
}