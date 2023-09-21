import React, { useState, useEffect } from 'react'
import './MqttIndicator.scss'
import logger from './logger'
import PropTypes from 'prop-types'
import useInterval from '@restart/hooks/useInterval'
import AsyncClient from 'async-mqtt'

function MqttIndicator (props) {
  const address = props.address
  const topic = props.topic
  const emoji = props.emoji

  let [blinking, setBlinking] = useState(false)
  let [started, setStarted] = useState(false)

  useEffect(() => {
    async function subscribe () {
      const mqtt = AsyncClient.connect(address)
      try {
        await mqtt.subscribe(topic)
        logger.log('subscribed mqttindicator ' + emoji + ' to topic ' + topic)
      } catch (e) {
        logger.error('error connecting to mqtt')
        logger.error(e)
      }

      // blink when a message comes in
      mqtt.on('message', function () {
        setBlinking(true)
      })
    }
    if (!started) {
      subscribe()
      setStarted(true)
    }
  }, [address, started, topic, emoji])

  // de-blink the indicator routinely
  useInterval(() => {
    setBlinking(false)
  }, 100)

  return (
    <div className={blinking ? 'indicator-blink' : 'indicator'}>
      <span role='img' aria-label='MQTT'>{emoji}</span>
    </div>
  )
}

MqttIndicator.propTypes = {
  address: PropTypes.string,
  topic: PropTypes.string,
  emoji: PropTypes.string
}

export { MqttIndicator }
