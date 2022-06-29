/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "polygontesselator.h"

#include "3rdparty/poly2tri/poly2tri.h"

QVector<QPointF> PolygonTessellator::tessellate(const QList<QPolygonF> &polygons)
{
    QVector<QPointF> triangles;

    auto triangulateSegment = [](const QList<QPolygonF> segment) {
        QVector<QPointF> ret;
        if (segment.isEmpty())
            return ret;

        QList<p2t::Point *> points;
        QList<std::vector<p2t::Point *>> polylines;

        QRectF segmentArea;
        for (int i = 0; i < segment.size(); i++) {
            const QPolygonF polygon = segment.at(i);
            const QRectF polygonRect = polygon.boundingRect();
            std::vector<p2t::Point *> polyline;
            for (int p = polygon.isClosed() ? 1 : 0; p < polygon.size(); p++) {
                const QPointF pt = polygon.at(p);
                points << new p2t::Point(pt.x(), pt.y());
                polyline.push_back(points.last());
            }

            if (!segmentArea.isNull() && segmentArea.contains(polygonRect))
                polylines.append(polyline);
            else {
                polylines.prepend(polyline);
                segmentArea = polygonRect;
            }
        }

        p2t::CDT cdt(polylines.first());
        for (int i = 1; i < polylines.size(); i++)
            cdt.AddHole(polylines.at(i));

        cdt.Triangulate();

        std::vector<p2t::Triangle *> tgls = cdt.GetTriangles();
        std::vector<p2t::Triangle *>::iterator it = tgls.begin();
        std::vector<p2t::Triangle *>::iterator end = tgls.end();
        while (it != end) {
            p2t::Triangle *tgl = *it;

            ret << QPointF(tgl->GetPoint(0)->x, tgl->GetPoint(0)->y);
            ret << QPointF(tgl->GetPoint(1)->x, tgl->GetPoint(1)->y);
            ret << QPointF(tgl->GetPoint(2)->x, tgl->GetPoint(2)->y);

            ++it;
        }

        qDeleteAll(points);

        return ret;
    };

    QRectF logicalSegmentArea;
    QList<QPolygonF> logicalSegment;

    for (const QPolygonF &polygon : polygons) {
        const QRectF polygonRect = polygon.boundingRect();

        if (logicalSegmentArea.isNull() || logicalSegmentArea.contains(polygonRect)
            || polygonRect.contains(logicalSegmentArea)) {
            logicalSegment.append(polygon);
            logicalSegmentArea |= polygonRect;
        } else {
            triangles += triangulateSegment(logicalSegment);

            logicalSegment.clear();
            logicalSegment.append(polygon);

            logicalSegmentArea = polygonRect;
        }
    }

    if (!logicalSegment.isEmpty())
        triangles += triangulateSegment(logicalSegment);

    return triangles;
}
