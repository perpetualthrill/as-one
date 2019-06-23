import React, { useState, useEffect } from 'react'
import './Indicators.scss'
import logger from './logger'
import PropTypes from 'prop-types'
import useInterval from '@restart/hooks/useInterval'
import match from 'mqtt-match'

function MqttIndicator (props) {
  const mqtt = props.mqtt
  const topic = props.topic
  const emoji = props.emoji

  let [blinking, setBlinking] = useState(false)
  let [started, setStarted] = useState(false)

  useEffect(() => {
    async function subscribe () {
      try {
        await mqtt.subscribe(topic)
        logger.log('subscribed mqttindicator '+emoji+' to topic '+topic)
      } catch (e) {
        logger.error('error connecting to mqtt')
        logger.error(e)
      }

      // blink when a message comes in
      mqtt.on('message', function (t) {
        // no idea why i have to filter these by hand. isn't that the whole
        // point of the notion of 'subscription'? anyhoo ...
        if (match(topic, t)) {
          setBlinking(true)
        }
      })
    }
    if (!started) {
      subscribe()
      setStarted(true)
    }
  }, [mqtt, started, topic, emoji])

  // de-blink the indicator routinely
  useInterval(() => {
    setBlinking(false)
  }, 50)

  return (
    <div className={blinking ? 'indicator-blink' : 'indicator'}>
      <span role='img' aria-label='MQTT'>{emoji}</span>
    </div>
  )
}

MqttIndicator.propTypes = {
  mqtt: PropTypes.object,
  topic: PropTypes.string,
  emoji: PropTypes.string
}

export { MqttIndicator }
