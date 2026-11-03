#pragma once

#include <QPointF>
#include <QSizeF>
#include <QTransform>

// What a map shows of the world, in plate carrée (equirectangular): its center, x the longitude and y the latitude in
// degrees, and its zoom. A degree takes as many pixels either way; at zoom 1, the whole world fits the viewport,
// centered. The center stays on the world, -180 to 180 degrees of longitude and -90 to 90 of latitude, and the zoom
// from MinZoom to MaxZoom: what is given beyond them is brought back to the nearest limit, what is not a number is
// ignored.
class MapView
{
  public:
    static constexpr double MinZoom = 1.0;
    static constexpr double MaxZoom = 64.0;

    MapView() = default;
    MapView(QPointF center, double zoom);

    [[nodiscard]] QPointF center() const;
    void setCenter(QPointF center);
    [[nodiscard]] double zoom() const;
    void setZoom(double zoom);

    // The pixels a degree takes in a viewport of that size.
    [[nodiscard]] double pixelsPerDegree(QSizeF viewport) const;
    // From degrees, x the longitude and y the latitude, to the pixels of the viewport, y downwards.
    [[nodiscard]] QTransform transform(QSizeF viewport) const;
    [[nodiscard]] QPointF toPixels(QPointF geographic, QSizeF viewport) const;
    [[nodiscard]] QPointF toGeographic(QPointF pixels, QSizeF viewport) const;

    // Moves the map by delta pixels, as the mouse drags it.
    void pan(QPointF delta, QSizeF viewport);
    // Multiplies the zoom by factor, the point of the world under pixels staying under it as far as the center can go.
    void zoomAt(QPointF pixels, double factor, QSizeF viewport);

  private:
    QPointF m_center;
    double m_zoom = MinZoom;
};
