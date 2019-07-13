import React, { useState } from 'react'

import { SensorMonitor } from './SensorMonitor'
import { MqttIndicator } from './MqttIndicator'
import { FireTest } from './FireTest'
import { ScoreboardEmulator, GREYISH_BLACK } from './ScoreboardEmulator'

import Container from 'react-bootstrap/Container'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'
import Navbar from 'react-bootstrap/Navbar'
import Collapse from 'react-bootstrap/Collapse'

const mqttAddress = 'ws://' + window.location.hostname + ':8181'

function App () {
  let [scoreboardOpen, setScoreboardOpen] = useState(true)
  let [sensorMonitorOpen, setSensorMonitorOpen] = useState(true)

  let spacer = <Row style={{ height: '40px' }} />

  return (
    <div className='App'>
      <Container fluid='true'>
        <Row>
          <Col md={0} lg={1} />
          <Col md={12} lg={10}>
            <Navbar expand='lg' variant='dark' bg='dark'>
              <Navbar.Brand href='#'>As One</Navbar.Brand>
              <div className='d-flex ml-auto flex-nowrap'>
                {/* flame effect heartbeat */}
                <MqttIndicator address={mqttAddress} topic='asOne/fe/heartbeat' emoji='🔥' />
                <div style={{ width: '10px' }} />
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
        { spacer }
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <div className='card' style={{ backgroundColor: GREYISH_BLACK }}>
              <h4 className='card-header'>
                Fire
              </h4>
              <div className='card-body'>
                <div className='card-text'><FireTest address={mqttAddress} /></div>
              </div>
            </div>
          </Col>
          <Col md={0} lg={1} />
        </Row>
        { spacer }
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <div className='card' style={{ backgroundColor: GREYISH_BLACK }}>
              <h4 className='card-header'>
                <a href='#/' onClick={() => setSensorMonitorOpen(!sensorMonitorOpen)}>
                  Sensors
                  <span className='float-right'>▼</span>
                </a>
              </h4>
              <Collapse in={sensorMonitorOpen}>
                <div className='card-body'>
                  <div className='card-text'>
                    <SensorMonitor address={mqttAddress} />
                  </div>
                </div>
              </Collapse>
            </div>
          </Col>
          <Col md={0} lg={1} />
        </Row>
        { spacer }
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <div className='card' style={{ backgroundColor: GREYISH_BLACK }}>
              <h4 className='card-header'>
                <a href='#/' onClick={() => setScoreboardOpen(!scoreboardOpen)}>
                Scoreboard
                  <span className='float-right'>▼</span>
                </a>
              </h4>
              <Collapse in={scoreboardOpen}>
                <div className='card-body'>
                  <div className='card-text'><ScoreboardEmulator address={mqttAddress} /></div>
                </div>
              </Collapse>
            </div>
          </Col>
          <Col md={0} lg={1} />
        </Row>

        { spacer }

      </Container>
    </div>
  )
}

export default App
