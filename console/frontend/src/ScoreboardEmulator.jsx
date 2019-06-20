import React, { useState, useEffect } from 'react'
import logger from './logger'
import PropTypes from 'prop-types'
import { Stage, Layer, Rect } from 'react-konva'
import { useClientRect } from './hooks'
import scoreboardAddressTable from './scoreboard-lookup.json'

const SCOREBOARD_WIDTH = 31
const SCOREBOARD_HEIGHT = 10
const GREYISH_BLACK = '#222222'
const DEFAULT_TINY_WIDTH_PX = 200

function ScoreboardEmulator (props) {
  const mqtt = props.mqtt

  let [started, setStarted] = useState(false)
  let [leds, setLeds] = useState([])
  let [board, setBoard] = useState([])
  let [checkedWidth, setCheckedWidth] = useState(DEFAULT_TINY_WIDTH_PX)
  let [rect, ref] = useClientRect()

  function _rgbToHex (r, g, b) {
    return '#' + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1)
  }

  function _locationToKey (x, y) {
    return 'key_' + x + '_' + y
  }

  function _pixelSize (divWidth) {
    return divWidth / SCOREBOARD_WIDTH
  }

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

  // Runs whenever new raw values are set. Updates the existing screen
  useEffect(() => {
    if (leds) {
      // First, process the incoming rgb values into an array of hex triplets
      const processedArray = []
      for (var i = 0; i < leds.length; i += 3) {
        const newColor = _rgbToHex(leds[i], leds[i + 1], leds[i + 2])
        processedArray.push(newColor)
      }

      // Then, use those to update the board
      const pixelSize = _pixelSize(checkedWidth)
      processedArray.map(function (value, index) {
        const x = scoreboardAddressTable[index].x
        const y = scoreboardAddressTable[index].y
        const replacement = <Rect
          x={pixelSize * x}
          y={pixelSize * y}
          width={pixelSize}
          height={pixelSize}
          fill={value}
          key={_locationToKey(x, y)} />
        board[x][y] = replacement
        return value
      })
      setBoard(board)
    }
  }, [leds, board, checkedWidth])

  // Runs whenever the screen is resized. Sets the width we're rendering to
  // and creates a screen using that size
  useEffect(() => {
    // tiny default so that we don't ever end up with a null screen
    var newCheckedWidth = 200
    if (rect) {
      newCheckedWidth = rect.width
    }
    setCheckedWidth(newCheckedWidth)

    // using that value, create the board
    const pixelSize = _pixelSize(newCheckedWidth)
    const screen = []
    for (var i = 0; i < SCOREBOARD_WIDTH; i++) {
      const column = []
      for (var j = 0; j < SCOREBOARD_HEIGHT; j++) {
        const pixel = <Rect x={pixelSize * i} y={pixelSize * j} width={pixelSize} height={pixelSize} fill={GREYISH_BLACK} key={_locationToKey(i, j)} />
        column.push(pixel)
      }
      screen.push(column)
    }
    setBoard(screen)
  }, [rect])

  return (
    <div ref={ref}>
      <Stage width={checkedWidth} height={checkedWidth / 3}>
        <Layer>
          { board.flat() }
        </Layer>
      </Stage>
    </div>
  )
}

ScoreboardEmulator.propTypes = {
  mqtt: PropTypes.object
}

export { ScoreboardEmulator }
