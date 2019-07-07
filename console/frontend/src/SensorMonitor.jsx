import React, { useState } from 'react'
import useInterval from '@restart/hooks/useInterval'
import axios from 'axios'
import logger from './logger'
import { SensorData } from './SensorData'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'
import PropTypes from 'prop-types'

const SENSOR_LIST_URL = '/sensors'

function SensorMonitor (props) {
  const address = props.address

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
      { Object.keys(sensorList).map(sensorName =>
        <Col lg={6} md key={sensorName}>
          <SensorData name={sensorName} address={address} />
        </Col>
      )}
    </Row>
  )
}

SensorMonitor.propTypes = {
  address: PropTypes.string
}

export { SensorMonitor }
