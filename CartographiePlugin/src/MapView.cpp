#include "MapView.h"

#include <algorithm>
#include <cmath>

namespace
{

constexpr double WorldWidth = 360.0;  // Degrees of longitude
constexpr double WorldHeight = 180.0; // Degrees of latitude

} // namespace

MapView::MapView(QPointF center, double zoom)
{
    setCenter(center);
    setZoom(zoom);
}

QPointF MapView::center() const
{
    return m_center;
}

void MapView::setCenter(QPointF center)
{
    if(!std::isfinite(center.x()) || !std::isfinite(center.y()))
    {
        return;
    }
    m_center = {std::clamp(center.x(), -WorldWidth / 2, WorldWidth / 2),
                std::clamp(center.y(), -WorldHeight / 2, WorldHeight / 2)};
}

double MapView::zoom() const
{
    return m_zoom;
}

void MapView::setZoom(double zoom)
{
    if(!std::isfinite(zoom))
    {
        return;
    }
    m_zoom = std::clamp(zoom, MinZoom, MaxZoom);
}

double MapView::pixelsPerDegree(QSizeF viewport) const
{
    const double fit = std::min(viewport.width() / WorldWidth, viewport.height() / WorldHeight);
    // An empty viewport shows nothing: any scale that can be inverted will do.
    return m_zoom * (fit > 0.0 ? fit : 1.0);
}

QTransform MapView::transform(QSizeF viewport) const
{
    const double scale = pixelsPerDegree(viewport);
    QTransform transform;
    transform.translate(viewport.width() / 2, viewport.height() / 2);
    transform.scale(scale, -scale); // North upwards
    transform.translate(-m_center.x(), -m_center.y());
    return transform;
}

QPointF MapView::toPixels(QPointF geographic, QSizeF viewport) const
{
    return transform(viewport).map(geographic);
}

QPointF MapView::toGeographic(QPointF pixels, QSizeF viewport) const
{
    return transform(viewport).inverted().map(pixels);
}

void MapView::pan(QPointF delta, QSizeF viewport)
{
    const double scale = pixelsPerDegree(viewport);
    setCenter({m_center.x() - delta.x() / scale, m_center.y() + delta.y() / scale});
}

void MapView::zoomAt(QPointF pixels, double factor, QSizeF viewport)
{
    const QPointF anchor = toGeographic(pixels, viewport);
    setZoom(m_zoom * factor);
    const double scale = pixelsPerDegree(viewport);
    const QPointF offset = pixels - QPointF(viewport.width() / 2, viewport.height() / 2);
    setCenter({anchor.x() - offset.x() / scale, anchor.y() + offset.y() / scale});
}
