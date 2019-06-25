import React from 'react'

import { SensorMonitor } from './SensorMonitor'
import { MqttIndicator } from './MqttIndicator'
import { ScoreboardEmulator } from './ScoreboardEmulator'

import Container from 'react-bootstrap/Container'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'
import Navbar from 'react-bootstrap/Navbar'

const mqttAddress = 'ws://' + window.location.hostname + ':8181'

function App () {
  return (
    <div className='App'>
      <Container fluid='true'>
        <Row>
          <Col md={0} lg={1} />
          <Col md={12} lg={10}>
            <Navbar expand='lg' variant='dark' bg='dark ' style={{ marginBottom: '40px' }}>
              <Navbar.Brand href='#'>As One</Navbar.Brand>
              <div className='d-flex ml-auto flex-nowrap'>
                {/* scoreboard heartbeat */}
                <MqttIndicator address={mqttAddress} topic='asOne/score/heartbeat' emoji='📺' />
                <div style={{ width: '10px' }} />
                {/* all messages, all topics */}
                <MqttIndicator address={mqttAddress} topic='asOne/#' emoji='📢' />
              </div>
            </Navbar>
          </Col>
          <Col md={0} lg={1} />
        </Row>
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <SensorMonitor />
          </Col>
          <Col md={0} lg={1} />
        </Row>
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <ScoreboardEmulator address={mqttAddress} />
          </Col>
          <Col md={0} lg={1} />
        </Row>
      </Container>
    </div>
  )
}

export default App
