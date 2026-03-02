export type RectSide = 'top' | 'right' | 'bottom' | 'left'
export type Vec2 = { x: number; y: number }
export type Box = {
  x: number
  y: number
  w: number
  h: number
}

/** Calculate the distance of two points. */
export function distanceOf(p1: Vec2, p2: Vec2): number {
  return Math.sqrt(Math.pow(p1.x - p2.x, 2) + Math.pow(p1.y - p2.y, 2))
}

/**
 * @param box
 * @param size Add `size` to all edges.
 */
export function growBox(box: Box, size: number): Box {
  return {
    x: box.x - size,
    y: box.y - size,
    w: box.w + 2 * size,
    h: box.h + 2 * size,
  }
}

export function isPointInBox(point: Vec2, box: Box): boolean {
  return (
    point.x > box.x &&
    point.x < box.x + box.w &&
    point.y > box.y &&
    point.y < box.y + box.h
  )
}

/** Calculate the control point. */
export function controlPointOf(
  target: Vec2,
  another: Vec2,
  sideOfTarget: RectSide,
  /**
   * The distance a control point must be far away from the target in the
   * direction of leaving the target.
   */
  minDistanceToTarget: number = 50
) {
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

/** Return the entering angle of a rectangle side. */
export function angleOf(enteringSide: RectSide): number {
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
