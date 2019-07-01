import React, { useState } from 'react'
import { useInterval } from './hooks'
import axios from 'axios'
import logger from './logger'
import { SensorData } from './SensorData'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'

const SENSOR_LIST_URL = '/sensors'

function SensorMonitor () {
  let [sensorList, setSensorList] = useState([])

  async function pollServerAndUpdate () {
    try {
      const response = await axios.get(SENSOR_LIST_URL)
      setSensorList(response.data)
    } catch (error) {
      logger.error(error)
    }
  }

  useInterval(() => {
    pollServerAndUpdate()
  }, 1000)

  return (
    <Row>
      { sensorList.map(sensorName =>
        <Col lg={6} md key={sensorName}>
          <SensorData url={'/sensors/' + sensorName} />
        </Col>
      )}
    </Row>
  )
}

export { SensorMonitor }
