import { useState, useEffect } from 'react'
import axios from 'axios'

// via https://stackoverflow.com/questions/56197689/hook-doesnt-rerender-component
// and https://medium.com/@cwlsn/how-to-fetch-data-with-react-hooks-in-a-minute-e0f9a15a44d6
function useFetch (url) {
  const [data, setData] = useState([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    async function fetchUrl () {
      try {
        const response = await axios.get(url);
        setData(response.data)
        setLoading(false)
      } catch (error) {
        // eslint-disable-next-line no-console
        console.error(error);
      }
    }

    fetchUrl()
  }, [url])
  return [data, loading]
}

export { useFetch }
