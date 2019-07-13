import React, { useState, useEffect } from 'react'
import PropTypes from 'prop-types'
import Button from 'react-bootstrap/Button'
import AsyncClient from 'async-mqtt'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'

function FireTest (props) {
  const address = props.address

  let [started, setStarted] = useState(false)
  let [mq, setMq] = useState(null)

  useEffect(() => {
    async function subscribe () {
      const client = AsyncClient.connect(address)
      setMq(client)
    }

    if (!started) {
      subscribe()
      setStarted(true)
    }
  }, [address, mq, started])

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
          onPointerDown={() => {
          // mqttPublish
          }}
          onPointerUp={() => {
            mqttPublish('asOne/fe/testUpper', '500')
          }}>Upper</Button>
      </Col>
      <Col>
        <Button
          label='Lower'
          id='lower-button'
          onPointerUp={() => {
            mqttPublish('asOne/fe/testLower', '375')
          }}>Lower</Button>
      </Col>
      <Col>
        <Button
          label='80 BPM'
          id='80-bpm-button'
          onPointerUp={() => {
            mqttPublish('asOne/fe/doBPM', '80')
          }}>80 BPM</Button>
      </Col>
      <Col>
        <Button
          label='Left BPM'
          id='left-bpm-button'
          onPointerUp={() => {
            mqttPublish('asOne/fe/doBPM', '80')
          }}>Left BPM ({})</Button>
      </Col>
      <Col />
      <Col />
    </Row>
  )
}

FireTest.propTypes = {
  address: PropTypes.string
}

export { FireTest }
