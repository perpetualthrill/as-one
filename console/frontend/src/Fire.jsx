import React from 'react'
import Button from 'react-bootstrap/Button'
import logger from './logger'
import axios from 'axios'

const FIRE_PATH = '/game/fire'

function Fire () {
  async function postToFire () {
    try {
      const response = await axios.post(FIRE_PATH)
      logger.log('fired effect: ' + response)
    } catch (error) {
      logger.error(error)
    }
  }

  return (
    <Button
      label='fire'
      style={{ width: '200px' }}
      id='fire-button'
      onPointerUp={() => {
        postToFire()
      }}>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;f&nbsp;i&nbsp;r&nbsp;e&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</Button>
  )
}

export { Fire }
