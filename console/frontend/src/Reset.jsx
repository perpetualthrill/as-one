import React from 'react'
import Button from 'react-bootstrap/Button'
import logger from './logger'
import axios from 'axios'

const RESET_PATH = '/game/reset'

function Reset () {
  async function postToReset () {
    try {
      const response = await axios.post(RESET_PATH)
      logger.log('reset effect: ' + response)
    } catch (error) {
      logger.error(error)
    }
  }

  return (
    <Button
      label='reset'
      id='reset-button'
      onClick={() => {
        postToReset()
      }}>reset</Button>
  )
}

export { Reset }
