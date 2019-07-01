import React, { useState, useEffect } from 'react'
import { useInterval, useCheckedWidth } from './hooks'
import axios from 'axios'
import logger from './logger'
import { Charts, ChartContainer, ChartRow, YAxis, LineChart } from 'react-timeseries-charts'
import { TimeSeries } from 'pondjs'
import PropTypes from 'prop-types'

SensorData.propTypes = {
  url: PropTypes.string.isRequired
}

function SensorData (props) {
  let [data, setData] = useState([])
  let [series, setSeries] = useState(null)
  let [ref, checkedWidth] = useCheckedWidth()

  function makeUnixDate (seconds, nanos) {
    var millis = seconds * 1000
    millis += nanos / 1000000
    return millis
  }

  useEffect(() => {
    let seriesData = {
      name: 'readings',
      columns: ['time', 's1', 's2', 's3', 's4'],
      points: []
    }
    if (data.length > 0) {
      let reversed = data.reverse()
      for (var reading of reversed) {
        if (reading == null) continue // not sure why, but this happened once and crashed the app, so
        let point = []
        let time = makeUnixDate(reading.timestamp.seconds, reading.timestamp.nanos)
        point.push(time)
        point.push(reading.s1)
        point.push(reading.s2)
        point.push(reading.s3)
        point.push(reading.s4)
        seriesData.points.push(point)
      }
      let series = new TimeSeries(seriesData)
      setSeries(series)
    }
  }, [data])

  async function pollServerAndUpdate () {
    try {
      const response = await axios.get(props.url)
      setData(response.data)
    } catch (error) {
      logger.error(error)
    }
  }

  useInterval(() => {
    pollServerAndUpdate()
  }, 250)

  const style = {
    s1: {
      stroke: '#a02c2c',
      opacity: 0.5
    },
    s2: {
      stroke: '#b03c3c',
      opacity: 0.5
    },
    s3: {
      stroke: '#c04c4c',
      opacity: 0.5
    },
    s4: {
      stroke: '#d05c5c',
      opacity: 0.5
    }
  }

  return (
    <div ref={ref}>
      { (series == null) ? '' : (
        <ChartContainer width={checkedWidth} timeRange={series.timerange()}>
          <ChartRow height={checkedWidth / 3} showGrid>
            <YAxis id='axis1' min={400} max={600} width={28} type='linear' format='.0f' />
            <Charts>
              <LineChart axis='axis1' series={series} columns={['s1', 's2', 's3', 's4']} style={style} />
            </Charts>
          </ChartRow>
        </ChartContainer>
      )}
    </div>
  )
}

export { SensorData }
