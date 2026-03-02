import * as React from 'react'
import { stylesheet } from 'typestyle'

interface Props {
  arrowDescriptor: number[]
  color?: string
  zIndex?: number
}

const styles = stylesheet({
  arrow: {
    position: 'absolute',
    top: 0,
    right: 0,
    pointerEvents: 'none',
    overflow: 'visible',
  },
})

export function Arrow({
  arrowDescriptor,
  color = 'black',
  zIndex = 0,
}: Props): JSX.Element {
  const arrowHeadSize = 9
  const [sx, sy, c1x, c1y, c2x, c2y, ex, ey, ae] = arrowDescriptor

  return (
    <svg
      className={styles.arrow}
      style={{ zIndex }}
      width="100%"
      height="100%"
      xmlns="http://www.w3.org/2000/svg">
      <path
        d={`M ${sx} ${sy} C ${c1x} ${c1y}, ${c2x} ${c2y}, ${ex} ${ey}`}
        stroke={color}
        strokeWidth={arrowHeadSize / 2}
        fill="none"
      />
      <polygon
        points={`0,${-arrowHeadSize} ${arrowHeadSize *
          2},0, 0,${arrowHeadSize}`}
        transform={`translate(${ex}, ${ey}) rotate(${ae})`}
        fill={color}
      />
    </svg>
  )
}
