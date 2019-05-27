import { useState, useEffect } from 'react'
import fetch from 'isomorphic-fetch'

// via https://stackoverflow.com/questions/56197689/hook-doesnt-rerender-component
// and https://medium.com/@cwlsn/how-to-fetch-data-with-react-hooks-in-a-minute-e0f9a15a44d6
function useFetch (url) {
  const [data, setData] = useState([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    async function fetchUrl () {
      const response = await fetch(url)
      const json = await response.json()
      setData(json)
      setLoading(false)
    }

    fetchUrl()
  }, [url])
  return [data, loading]
}

export { useFetch }
