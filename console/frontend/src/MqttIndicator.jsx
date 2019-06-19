import React, { useState, useEffect } from 'react'
import './Indicators.css'
import Navbar from 'react-bootstrap/Navbar'

function MqttIndicator(props) {
    return (
        <div className="indicator">
            <span role="img" aria-label="MQTT">📢</span>
        </div>
    )
}

export { MqttIndicator }
