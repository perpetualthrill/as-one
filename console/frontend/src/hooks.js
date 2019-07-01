import { useState, useEffect, useRef, useCallback } from 'react'
import axios from 'axios'
import logger from './logger'

// via https://stackoverflow.com/questions/56197689/hook-doesnt-rerender-component
// and https://medium.com/@cwlsn/how-to-fetch-data-with-react-hooks-in-a-minute-e0f9a15a44d6
function useFetch (url) {
  const [data, setData] = useState([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    async function fetchUrl () {
      try {
        const response = await axios.get(url)
        setData(response.data)
        setLoading(false)
      } catch (error) {
        logger.error(error)
      }
    }

    fetchUrl()
  }, [url])
  return [data, loading]
}

// via https://overreacted.io/making-setinterval-declarative-with-react-hooks/
function useInterval (callback, delay) {
  const savedCallback = useRef()

  // Remember the latest callback.
  useEffect(() => {
    savedCallback.current = callback
  }, [callback])

  // Set up the interval.
  useEffect(() => {
    function tick () {
      savedCallback.current()
    }
    if (delay !== null) {
      let id = setInterval(tick, delay)
      return () => clearInterval(id)
    }
  }, [delay])
}

// via https://reactjs.org/docs/hooks-faq.html#how-can-i-measure-a-dom-node
// hacked to attend to window resize events
function useClientRect () {
  const [rect, setRect] = useState(null)

  const ref = useCallback(node => {
    function update () {
      if (node !== null) {
        setRect(node.getBoundingClientRect())
      }
    }
    window.addEventListener('resize', update)
    update()

    return () => {
      window.removeEventListener('resize', update)
    }
  }, [])

  return [rect, ref]
}

const DEFAULT_TINY_WIDTH_PX = 200
function useCheckedWidth () {
  let [checkedWidth, setCheckedWidth] = useState(DEFAULT_TINY_WIDTH_PX)
  let [rect, ref] = useClientRect()

  useEffect(() => {
    // tiny default so that we don't ever end up with a null screen
    var newCheckedWidth = DEFAULT_TINY_WIDTH_PX
    if (rect) {
      newCheckedWidth = rect.width
    }
    setCheckedWidth(newCheckedWidth)
  }, [rect, checkedWidth])

  return [ref, checkedWidth, setCheckedWidth]
}

export { useFetch, useInterval, useClientRect, useCheckedWidth }
