import React, { useState, useEffect } from 'react'
import './Indicators.scss'
import logger from './logger'
import PropTypes from 'prop-types'
import useInterval from '@restart/hooks/useInterval'

function MqttIndicator (props) {
  const mqtt = props.mqtt

  let [blinking, setBlinking] = useState(false)
  let [started, setStarted] = useState(false)

  useEffect(() => {
    async function subscribe () {
      try {
        await mqtt.subscribe('asOne/#')
        logger.log('subscribed mqttindicator')
        await mqtt.publish('asOne/hello', 'hello from console')
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
  }, [mqtt, started])

  // de-blink the indicator routinely
  useInterval(() => {
    setBlinking(false)
  }, 50)

  return (
    <div className={blinking ? 'indicator-blink' : 'indicator'}>
      <span role='img' aria-label='MQTT'>📢</span>
    </div>
  )
}

MqttIndicator.propTypes = {
  mqtt: PropTypes.object
}

export { MqttIndicator }
