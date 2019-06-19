import React, { useState, useEffect } from 'react'
import logger from './logger'
import PropTypes from 'prop-types'
import { Stage, Layer, Rect } from 'react-konva'
import { useClientRect } from './hooks'

function ScoreboardEmulator (props) {
  const mqtt = props.mqtt

  let [started, setStarted] = useState(false)
  let [leds, setLeds] = useState([])
  let [processedArray, setProcessedArray] = useState([])

  const [rect, ref] = useClientRect()

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
        // var newColor = [leds[i], leds[i + 1], leds[i + 2]]
        var newColor = '#' + _rgbToHex(leds[i], leds[i + 1], leds[i + 2])
        newProcessed.push(newColor)
      }
      setProcessedArray(newProcessed)
    }
  }, [leds])

  function _rgbToHex (r, g, b) {
    return ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1)
  }

  // make sure we do not pass a null rect into render
  var checkedWidth = 200
  if (rect) {
    checkedWidth = rect.width
  }
  // scoreboard is logically 31 pixels wide by 10 pixels tall
  const pixelSize = checkedWidth / 31

  return (
    <div ref={ref}>
      <Stage width={checkedWidth} height={checkedWidth / 3}>
        <Layer>
          { processedArray.map(function (value, index) {
            var x = 0; var y = 0
            for (var i = 0; i < index; i++) {
              x += pixelSize
              if (x > checkedWidth) {
                x = 0
                y += pixelSize
              }
            }
            return <Rect x={x} y={y} width={pixelSize} height={pixelSize} fill={value} key={index} />
          })}
        </Layer>
      </Stage>
    </div>
  )
}

ScoreboardEmulator.propTypes = {
  mqtt: PropTypes.object
}

export { ScoreboardEmulator }
