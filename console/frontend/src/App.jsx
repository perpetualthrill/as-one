import React from 'react'
import './App.css'
import { SensorData } from './SensorData'
import Container from 'react-bootstrap/Container'
import Row from 'react-bootstrap/Row'
import Col from 'react-bootstrap/Col'
import Navbar from 'react-bootstrap/Navbar'

function App () {
  return (
    <div className='App'>
      <Container fluid='true'>
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <Navbar expand='lg' variant='dark' bg='dark ' style={{ marginBottom: '40px' }}>
              <Navbar.Brand href='#'>As One</Navbar.Brand>
            </Navbar>
          </Col>
          <Col md={0} lg={1} />
        </Row>
        <Row>
          <Col md={0} lg={1} />
          <Col>
            <SensorData />
          </Col>
          <Col md={0} lg={1} />
        </Row>
      </Container>
    </div>
  )
}

export default App
