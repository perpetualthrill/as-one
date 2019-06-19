import React, { useState, useEffect } from 'react'
import logger from './logger'
import PropTypes from 'prop-types'

function ScoreboardEmulator (props) {
  const mqtt = props.mqtt

  let [started, setStarted] = useState(false)
  let [leds, setLeds] = useState([])
  let [processedArray, setProcessedArray] = useState([])

  // Should run only at mount time
  useEffect(() => {
    async function subscribe () {
      try {
        await mqtt.subscribe('asOne/score/all/direct')
        logger.log('subscribed scoreboardemulator')
      } catch (e) {
        logger.error('error connecting to mqtt')
        logger.error(e)
      }

      mqtt.on('message', function (_, message) {
        setLeds(message)
      })
    }
    if (!started) {
      subscribe()
      setStarted(true)
    }
  }, [mqtt, started])

  // Runs whenever new raw values are set
  useEffect(() => {
    if (leds) {
      var newProcessed = []
      for (var i = 0; i < leds.length; i += 3) {
        var newColor = [leds[i], leds[i + 1], leds[i + 2]]
        newProcessed.push(newColor)
      }
      setProcessedArray(newProcessed)
    }
  }, [leds])

  return (
    <div>helo frend: {
      processedArray.map(function (value, index) {
        return <div key={index}> {index} - {value}</div>
      })
    }
    </div>
  )
}

ScoreboardEmulator.propTypes = {
  mqtt: PropTypes.object
}

export { ScoreboardEmulator }
