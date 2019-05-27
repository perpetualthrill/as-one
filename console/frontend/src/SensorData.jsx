import React, { useState } from 'react'
import { useInterval } from './hooks'
import axios from 'axios'
import logger from './logger'

function SensorData () {
  const URL_LATEST = '/sensors/latest'

  let [data, setData] = useState([])
  let [loading, setLoading] = useState(true)

  async function pollServerAndUpdate () {
    try {
      const response = await axios.get(URL_LATEST)
      setData(response.data)
      setLoading(false)
    } catch (error) {
      logger.error(error)
    }
  }

  useInterval(() => {
    pollServerAndUpdate()
  }, 1000)

  return (
    <>
      <p>hello sensors</p>
      {loading ? ('Loading ...') : (
        <p>{JSON.stringify(data)}</p>
      )}
    </>
  )
}

export { SensorData }
