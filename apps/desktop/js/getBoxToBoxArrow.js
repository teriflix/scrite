/**
  This code is created from the npm-package available at
  this git repository: https://github.com/dragonman225/curved-arrows
  A complete copy of the code is available in 3rdparty folder.

  The npm-package is written in TypeScript. Qt only support JavaScript,
  so much of the "typing" is removed from this code.
  */

function distanceOf(p1, p2) {
  return Math.sqrt(Math.pow(p1.x - p2.x, 2) + Math.pow(p1.y - p2.y, 2))
}

function growBox(box, size) {
  return {
    x: box.x - size,
    y: box.y - size,
    w: box.w + 2 * size,
    h: box.h + 2 * size,
  }
}

function isPointInBox(point, box) {
  return (
    point.x > box.x &&
    point.x < box.x + box.w &&
    point.y > box.y &&
    point.y < box.y + box.h
  )
}

function controlPointOf( target, another, sideOfTarget, minDistanceToTarget ) {
  if(minDistanceToTarget === undefined)
    minDistanceToTarget = 50

  switch (sideOfTarget) {
    case 'top': {
      return {
        x: target.x,
        y: Math.min((target.y + another.y) / 2, target.y - minDistanceToTarget),
      }
    }
    case 'bottom': {
      return {
        x: target.x,
        y: Math.max((target.y + another.y) / 2, target.y + minDistanceToTarget),
      }
    }
    case 'left': {
      return {
        x: Math.min((target.x + another.x) / 2, target.x - minDistanceToTarget),
        y: target.y,
      }
    }
    case 'right': {
      return {
        x: Math.max((target.x + another.x) / 2, target.x + minDistanceToTarget),
        y: target.y,
      }
    }
  }
}

function angleOf(enteringSide) {
  switch (enteringSide) {
    case 'left':
      return 0
    case 'top':
      return 90
    case 'right':
      return 180
    case 'bottom':
      return 270
  }
}

function getBoxToBoxArrow( x0, y0, w0, h0, x1, y1, w1, h1, userOptions ) {
  const options = {
    padStart: userOptions.padStart ? userOptions.padStart : 0,
    padEnd: userOptions.padEnd ? userOptions.padEnd : 0,
    controlPointStretch: userOptions.controlPointStretch ? userOptions.controlPointStretch : 50
  }

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

  const sides = ['top', 'right', 'bottom', 'left']
  const startPoints = [startAtTop, startAtRight, startAtBottom, startAtLeft]
  const endPoints = [endAtTop, endAtRight, endAtBottom, endAtLeft]

  let shortestDistance = 1 / 0
  let bestStartPoint = startAtTop
  let bestEndPoint = endAtTop
  let bestStartSide = 'top'
  let bestEndSide = 'top'

  const keepOutZone = 15
  for (let startSideId = 0; startSideId < sides.length; startSideId++) {
    const startPoint = startPoints[startSideId]
    if (isPointInBox(startPoint, growBox(endBox, keepOutZone))) continue

    for (let endSideId = 0; endSideId < sides.length; endSideId++) {
      const endPoint = endPoints[endSideId]

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
