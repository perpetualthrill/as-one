package org.perpetualthrill.asone.console.util

import org.slf4j.LoggerFactory

fun Any.logInfo(message: String) {
    val logger = LoggerFactory.getLogger(this.javaClass.name)
    logger.info(message)
}