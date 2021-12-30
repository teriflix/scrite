# Curved Arrows

![version](https://img.shields.io/npm/v/curved-arrows.svg?style=flat-square&color=007acc&label=version) ![license](https://img.shields.io/github/license/dragonman225/notablog.svg?style=flat-square&label=license&color=08CE5D)

A set of functions for drawing S-curved arrows between points and shapes.

- [`getArrow`](#getarrowx0-y0-x1-y1-options) - For point-to-point arrows (**has glitches, not ready yet**).
- [`getBoxToBoxArrow`](#getboxtoboxarrowx0-y0-w0-h0-x1-y1-w1-h1-options) - For rectangle-to-rectangle arrows.

![demo animation](./demo_animation.gif)

[👉 Demo](https://dragonman225.js.org/p/curved-arrows/) | [How it works](https://dragonman225.js.org/curved-arrows.html)

You may also want to see [steveruizok's perfect-arrows](https://github.com/steveruizok/perfect-arrows), it's smarter but the start point and the end point are less predictable.

## Installation

```bash
npm i curved-arrows
```

_or_

```bash
yarn add curved-arrows
```

## Usage

The functions in this library has similar arguments and return values to [steveruizok's perfect-arrows](https://github.com/steveruizok/perfect-arrows). Notable differences are options and the return values containing two control points, one for start point and one end point, instead of one, to represent an S-curve.

The return values provide only the information needed to draw an arrow. You'll need to draw the arrow yourself using your technology of choice. See below for [an example using React and SVG](#example-a-react-arrow-component).

### `getArrow(x0, y0, x1, y1, options)`

The `getArrow` function accepts the position of two points and returns an array containing this information:

- four points: start, end, and two control points (one for start, one for end)
- two angles: end and start

You can use this information to draw an S-curve and arrow heads. You can use the options object to tweak the return values.

```js
const arrowHeadSize = 9
const arrow = getArrow(0, 0, 100, 200, {
  padStart: 0,
  padEnd: arrowHeadSize,
})
const [sx, sy, c1x, c1y, c2x, c2y, ex, ey, ae, as] = arrow
```

#### Arguments

| Argument | Type   | Description                                                                 |
| -------- | ------ | --------------------------------------------------------------------------- |
| `x0`     | number | The x position of the starting point.                                       |
| `y0`     | number | The y position of the starting point.                                       |
| `x1`     | number | The x position of the ending point.                                         |
| `y1`     | number | The y position of the ending point.                                         |
| `options`| object | An (optional) object containing one or more of the options described below. |

#### Options

| Option     | Type   | Default | Description                                                  |
| ---------- | ------ | ------- | ------------------------------------------------------------ |
| `padStart` | number | 0       | How far the arrow's starting point should be from the provided start point. |
| `padEnd`   | number | 0       | How far the arrow's ending point should be from the provided end point. |

#### Returns

| Argument | Type   | Description                                      |
| -------- | ------ | ------------------------------------------------ |
| `sx`     | number | The x position of the (padded) starting point.   |
| `sy`     | number | The y position of the (padded) starting point.   |
| `c1x`    | number | The x position of the control point of the starting point.   |
| `c1y`    | number | The y position of the control point of the starting point.   |
| `c2x`    | number | The x position of the control point of the ending point.     |
| `c2y`    | number | The y position of the control point of the ending point.     |
| `ex`     | number | The x position of the (padded) ending point.     |
| `ey`     | number | The y position of the (padded) ending point.     |
| `ae`     | number | The angle (in degree) for an ending arrowhead.   |
| `as`     | number | The angle (in degree) for a starting arrowhead.  |

---

### `getBoxToBoxArrow(x0, y0, w0, h0, x1, y1, w1, h1, options)`

The `getBoxToBoxArrow` function accepts the position and dimensions of two boxes (or rectangles) and returns an array containing this information:

- four points: start, end, and two control points (one for start, one for end)
- two angles: end and start

You can use this information to draw an S-curve and arrow heads. You can use the options object to tweak the return values.

**Note:** The options and values returned by `getBoxToBoxArrow` are in the same format as the options and values for `getArrow`.

```js
const arrowHeadSize = 9
const arrow = getArrow(0, 0, 200, 100, 300, 50, 200, 100, {
  padStart: 0,
  padEnd: arrowHeadSize,
})
const [sx, sy, c1x, c1y, c2x, c2y, ex, ey, ae, as] = arrow
```

#### Arguments

| Argument | Type   | Description                                                                 |
| -------- | ------ | --------------------------------------------------------------------------- |
| `x0`     | number | The x position of the first rectangle.                                      |
| `y0`     | number | The y position of the first rectangle.                                      |
| `w0`     | number | The width of the first rectangle.                                           |
| `h0`     | number | The height of the first rectangle.                                          |
| `x1`     | number | The x position of the second rectangle.                                     |
| `y1`     | number | The y position of the second rectangle.                                     |
| `w1`     | number | The width of the second rectangle.                                          |
| `h1`     | number | The height of the second rectangle.                                         |
| `options`| object | An (optional) object containing one or more of the options described below. |

#### Options

See options in `getArrow` above. (Both functions use the same options object.)

#### Returns

See returns in `getArrow` above. (Both functions return the same set of values.)

## Example: A React Arrow Component

```jsx
import * as React from 'react'
import { getArrow } from 'curved-arrows'

export function Arrow() {
  const p1 = { x: 100, y: 100 }
  const p2 = { x: 300, y: 200 }
  const arrowHeadSize = 9
  const color = 'black'
  const [sx, sy, c1x, c1y, c2x, c2y, ex, ey, ae] = getArrow(p1.x, p1.y, p2.x, p2.y, {
    padEnd: arrowHeadSize,
  })

  return (
    <svg
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
```

## Author

[@dragonman225](https://twitter.com/dragonman225)