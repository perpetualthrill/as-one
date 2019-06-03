import React, { useState, useEffect } from 'react'
import { useInterval } from './hooks'
import axios from 'axios'
import logger from './logger'
import { SensorData } from './SensorData'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'

function SensorMonitor () {
  const SENSOR_LIST_URL = '/sensors'

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

  useEffect(() => {
  })

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
