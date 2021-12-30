import * as React from 'react'
import { stylesheet } from 'typestyle'

interface Props {
  x1: number
  y1: number
  x2: number
  y2: number
  color?: string
  strokeWidth?: number
  zIndex?: number
}

const styles = stylesheet({
  line: {
    position: 'absolute',
    top: 0,
    right: 0,
    pointerEvents: 'none',
    overflow: 'visible',
  },
})

export function Line({
  x1,
  y1,
  x2,
  y2,
  color = 'black',
  strokeWidth = 3,
  zIndex = 0,
}: Props) {
  return (
    <svg
      className={styles.line}
      style={{ zIndex }}
      width="100%"
      height="100%"
      xmlns="http://www.w3.org/2000/svg">
      <path
        d={`M ${x1} ${y1} L ${x2} ${y2}`}
        stroke={color}
        strokeWidth={strokeWidth}
        fill="none"
      />
    </svg>
  )
}
