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
    sensorList.map(sensorName =>
      <Row key={sensorName}>
        <Col md={0} lg={1} />
        <Col>
          <SensorData url={'/sensors/' + sensorName} />
        </Col>
        <Col md={0} lg={1} />
      </Row>
    )
  )
}

export { SensorMonitor }
