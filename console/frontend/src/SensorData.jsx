import React, { useState, useEffect } from 'react'
import { useInterval } from './hooks'
import axios from 'axios'
import logger from './logger'
import { Charts, ChartContainer, ChartRow, YAxis, LineChart } from 'react-timeseries-charts'
import { TimeSeries } from 'pondjs'

function SensorData () {
  const URL_LATEST = '/sensors/latest'

  let [data, setData] = useState([])
  let [loading, setLoading] = useState(true)
  let [series, setSeries] = useState(null)

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
      const response = await axios.get(URL_LATEST)
      setData(response.data)
      setLoading(false)
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
      stroke: '#a03c3c',
      opacity: 0.5
    },
    s3: {
      stroke: '#a04c4c',
      opacity: 0.5
    },
    s4: {
      stroke: '#a05c5c',
      opacity: 0.5
    }
  }

  return (
    <>
      { loading || (series == null) ? ('Loading ...') : (

        <ChartContainer
          timeRange={series.timerange()}
          width={800}
        >
          <ChartRow height='300' showGrid>
            <YAxis id='axis1' min={300} max={800} width='40' type='linear' format='.0f' />
            <Charts>
              <LineChart axis='axis1' series={series} columns={['s1', 's2', 's3', 's4']} style={style} />
            </Charts>
          </ChartRow>
        </ChartContainer>
      )}
    </>
  )
}

export { SensorData }
