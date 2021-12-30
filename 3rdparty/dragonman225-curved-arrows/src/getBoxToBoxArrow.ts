import {
  angleOf,
  controlPointOf,
  distanceOf,
  growBox,
  isPointInBox,
  RectSide,
} from './utils'

/** Parameters that describe an arrow. */
export type ArrowDescriptor = [
  /** start point */
  /* sx: */ number,
  /* sy: */ number,
  /** control point for start point */
  /* c1x: */ number,
  /* c1y: */ number,
  /** control point for end point */
  /* c2x: */ number,
  /* c2y: */ number,
  /** end point */
  /* ex: */ number,
  /* ey: */ number,
  /** angle of end point */
  /* ae: */ number,
  /** angle of start point */
  /* as: */ number
]

export type ArrowOptions = Partial<{
  padStart: number
  padEnd: number
  controlPointStretch: number
}>

/**
 * Get parameters to draw an S-curved line between two boxes.
 *
 * @returns [sx, sy, c1x, c1y, c2x, c2y, ex, ey, ae, as]
 * @example
 * const arrowHeadSize = 9
 * const [
 *  startX, startY,
 *  controlStartX, controlStartY,
 *  controlEndX, controlEndY,
 *  endX, endY,
 *  endAngle,
 *  startAngle
 * ] = getBoxToBoxArrow(0, 0, 100, 100, 200, 200, 200, 100, {
 *   padStart: 0,
 *   padEnd: arrowHeadSize, // make room for drawing arrow head
 * })
 */
export default function getBoxToBoxArrow(
  /** start box */
  x0: number,
  y0: number,
  w0: number,
  h0: number,
  /** end box */
  x1: number,
  y1: number,
  w1: number,
  h1: number,
  userOptions?: ArrowOptions
): ArrowDescriptor {
  const options = {
    padStart: 0,
    padEnd: 0,
    controlPointStretch: 50,
    ...userOptions,
  }

  /** Points of start box. */
  const startBox = { x: x0, y: y0, w: w0, h: h0 }
  const startAtTop = {
    x: x0 + w0 / 2,
    y: y0 - 2 * options.padStart,
  }
  const startAtBottom = {
    x: x0 + w0 / 2,
    y: y0 + h0 + 2 * options.padStart,
  }
  const startAtLeft = {
    x: x0 - 2 * options.padStart,
    y: y0 + h0 / 2,
  }
  const startAtRight = {
    x: x0 + w0 + 2 * options.padStart,
    y: y0 + h0 / 2,
  }

  /** Points of end box. */
  const endBox = { x: x1, y: y1, w: w1, h: h1 }
  const endAtTop = { x: x1 + w1 / 2, y: y1 - 2 * options.padEnd }
  const endAtBottom = {
    x: x1 + w1 / 2,
    y: y1 + h1 + 2 * options.padEnd,
  }
  const endAtLeft = { x: x1 - 2 * options.padEnd, y: y1 + h1 / 2 }
  const endAtRight = {
    x: x1 + w1 + 2 * options.padEnd,
    y: y1 + h1 / 2,
  }

  const sides: RectSide[] = ['top', 'right', 'bottom', 'left']
  const startPoints = [startAtTop, startAtRight, startAtBottom, startAtLeft]
  const endPoints = [endAtTop, endAtRight, endAtBottom, endAtLeft]

  let shortestDistance = 1 / 0
  let bestStartPoint = startAtTop
  let bestEndPoint = endAtTop
  let bestStartSide: RectSide = 'top'
  let bestEndSide: RectSide = 'top'

  const keepOutZone = 15
  for (let startSideId = 0; startSideId < sides.length; startSideId++) {
    const startPoint = startPoints[startSideId]
    if (isPointInBox(startPoint, growBox(endBox, keepOutZone))) continue

    for (let endSideId = 0; endSideId < sides.length; endSideId++) {
      const endPoint = endPoints[endSideId]

      /**
       * If the start point is in the rectangle of end, or the end point
       * is in the rectangle of start, this combination is abandoned.
       */
      if (isPointInBox(endPoint, growBox(startBox, keepOutZone))) continue

      const d = distanceOf(startPoint, endPoint)
      if (d < shortestDistance) {
        shortestDistance = d
        bestStartPoint = startPoint
        bestEndPoint = endPoint
        bestStartSide = sides[startSideId]
        bestEndSide = sides[endSideId]
      }
    }
  }

  const controlPointForStartPoint = controlPointOf(
    bestStartPoint,
    bestEndPoint,
    bestStartSide,
    options.controlPointStretch
  )
  const controlPointForEndPoint = controlPointOf(
    bestEndPoint,
    bestStartPoint,
    bestEndSide,
    options.controlPointStretch
  )

  return [
    bestStartPoint.x,
    bestStartPoint.y,
    controlPointForStartPoint.x,
    controlPointForStartPoint.y,
    controlPointForEndPoint.x,
    controlPointForEndPoint.y,
    bestEndPoint.x,
    bestEndPoint.y,
    angleOf(bestEndSide),
    angleOf(bestStartSide),
  ]
}
