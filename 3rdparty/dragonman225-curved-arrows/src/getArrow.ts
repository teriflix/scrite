import getBoxToBoxArrow, {
  ArrowDescriptor,
  ArrowOptions,
} from './getBoxToBoxArrow'

/**
 * Get parameters to draw an S-curved line between two points.
 * 
 * @param x0 
 * @param y0 
 * @param x1 
 * @param y1 
 * @param userOptions 
 * @returns [sx, sy, c1x, c1y, c2x, c2y, ex, ey, ae, as]
 * @example
 * const arrowHeadSize = 9
 * const arrow = getArrow(0, 0, 100, 200, {
 *   padStart: 0,
 *   padEnd: arrowHeadSize,
 * })
 * const [sx, sy, c1x, c1y, c2x, c2y ex, ey, ae, as] = arrow
 */
export default function getArrow(
  x0: number,
  y0: number,
  x1: number,
  y1: number,
  userOptions?: ArrowOptions
): ArrowDescriptor {
  return getBoxToBoxArrow(x0, y0, 0, 0, x1, y1, 0, 0, userOptions)
}
