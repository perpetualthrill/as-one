import React, { useState, useEffect } from 'react'
import PropTypes from 'prop-types'
import Button from 'react-bootstrap/Button'
import AsyncClient from 'async-mqtt'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'
import logger from './logger'
import { Reset } from './Reset'

function FireTest (props) {
  const address = props.address

  let [started, setStarted] = useState(false)
  let [mq, setMq] = useState(null)
  let [lefty, setLefty] = useState('169')

  useEffect(() => {
    async function subscribe () {
      const client = AsyncClient.connect(address)
      try {
        await client.subscribe('asOne/console/leftBPM')
      } catch (e) {
        logger.error('error connecting to mqtt')
        logger.error(e)
      }

      client.on('message', (_, message) => {
        if (message == null) return
        const msgString = message.toString()
        if (msgString) {
          setLefty(msgString)
        }
      })

      setMq(client)
    }

    if (!started) {
      subscribe()
      setStarted(true)
    }
  }, [address, mq, started, setLefty])

  function mqttPublish (topic, string) {
    const mqtt = mq
    if (mqtt && mqtt.publish) {
      mqtt.publish(topic, string)
    }
  }

  return (
    <Row>
      <Col>
        <Button
          label='Upper'
          id='upper-button'
          onClick={() => {
            mqttPublish('asOne/fe/testUpper', '500')
          }}>Upper ______</Button>
      </Col>
      <Col>
        <Button
          label='Lower'
          id='lower-button'
          onClick={() => {
            mqttPublish('asOne/fe/testLower', '375')
          }}>Lower ______</Button>
      </Col>
      <Col>
        <Button
          label='80 BPM'
          id='80-bpm-button'
          onClick={() => {
            mqttPublish('asOne/fe/doBPM', '80')
          }}>80 BPM ______</Button>
      </Col>
      <Col>
        <Button
          label='Left BPM'
          id='left-bpm-button'
          onClick={() => {
            mqttPublish('asOne/fe/doBPM', lefty)
          }}>Left BPM ({lefty})</Button>
      </Col>
      <Col><Reset /></Col>
      <Col />
    </Row>
  )
}

FireTest.propTypes = {
  address: PropTypes.string
}

export { FireTest }
