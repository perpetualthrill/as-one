import React from 'react'
import { useFetch } from './hooks'

function SensorData () {
  const [data, loading] = useFetch('/sensors/latest')

  return (
    <>
      <p>hello sensors</p>
      {loading ? ('Loading ...') : (
        <p>{JSON.stringify(data)}</p>
      )}
    </>
  )
}

export { SensorData }
