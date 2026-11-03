#include "MapWidget.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <array>
#include <cmath>
#include <utility>

#include "Basemap.h"
#include "MapStyle.h"

namespace
{

// The world, in degrees, x the longitude and y the latitude.
const QRectF WorldRect(-180.0, -90.0, 360.0, 180.0);

// The spacing of the meridians and parallels, in degrees: the finest of these that leaves MinSpacing pixels between two
// lines, the coarsest when none does.
double graticuleStep(double pixelsPerDegree)
{
    constexpr double MinSpacing = 60.0;
    constexpr std::array Steps{0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0};
    for(const double step : Steps)
    {
        if(step * pixelsPerDegree >= MinSpacing)
        {
            return step;
        }
    }
    return Steps.back();
}

QString degrees(double value)
{
    return QString::number(std::abs(value), 'g', 6) + QChar(0x00B0);
}

// 30°E, 15°W; neither the prime meridian nor the antimeridian has a hemisphere.
QString longitudeLabel(double longitude)
{
    if(longitude == 0.0 || std::abs(longitude) == 180.0)
    {
        return degrees(longitude);
    }
    return degrees(longitude) + (longitude > 0.0 ? u'E' : u'W');
}

// 45°N, 15°S; the equator has no hemisphere.
QString latitudeLabel(double latitude)
{
    if(latitude == 0.0)
    {
        return degrees(latitude);
    }
    return degrees(latitude) + (latitude > 0.0 ? u'N' : u'S');
}

// In the middle of a pixel, so that a line one pixel wide is drawn sharp.
double pixelCenter(double coordinate)
{
    return std::floor(coordinate) + 0.5;
}

} // namespace

MapWidget::MapWidget(QWidget* parent) : QWidget(parent), m_land(Basemap::world().land())
{
    setAttribute(Qt::WA_OpaquePaintEvent); // It paints all of itself
    setCursor(Qt::OpenHandCursor);
}

double MapWidget::longitude() const
{
    return m_view.center().x();
}

void MapWidget::setLongitude(double longitude)
{
    MapView view = m_view;
    view.setCenter({longitude, m_view.center().y()});
    setView(view);
}

double MapWidget::latitude() const
{
    return m_view.center().y();
}

void MapWidget::setLatitude(double latitude)
{
    MapView view = m_view;
    view.setCenter({m_view.center().x(), latitude});
    setView(view);
}

double MapWidget::zoom() const
{
    return m_view.zoom();
}

void MapWidget::setZoom(double zoom)
{
    MapView view = m_view;
    view.setZoom(zoom);
    setView(view);
}

QSize MapWidget::sizeHint() const
{
    return {720, 360}; // Two pixels a degree at zoom 1
}

QSize MapWidget::minimumSizeHint() const
{
    return {180, 90};
}

void MapWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor::fromRgba(MapStyle::Outside));

    const QTransform transform = m_view.transform(size());
    const QRectF world = transform.mapRect(WorldRect);
    painter.fillRect(world, QColor::fromRgba(MapStyle::Sea));

    QPen coastline(QColor::fromRgba(MapStyle::Coastline), 1.0);
    coastline.setCosmetic(true); // One pixel wide whatever the zoom
    painter.save();
    painter.setTransform(transform);
    painter.setPen(coastline);
    painter.setBrush(QColor::fromRgba(MapStyle::Land));
    painter.drawPath(m_land);
    painter.restore();

    paintGraticule(painter);

    painter.setPen(coastline);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(world);
}

void MapWidget::paintGraticule(QPainter& painter) const
{
    const QSizeF viewport = size();
    const QRectF visible = m_view.transform(viewport).mapRect(WorldRect).intersected(QRectF(rect()));
    if(visible.isEmpty())
    {
        return;
    }
    const QPointF northWest = m_view.toGeographic(visible.topLeft(), viewport);
    const QPointF southEast = m_view.toGeographic(visible.bottomRight(), viewport);
    const double step = graticuleStep(m_view.pixelsPerDegree(viewport));

    QPen line(QColor::fromRgba(MapStyle::Graticule), 1.0);
    line.setCosmetic(true);
    const QPen label(QColor::fromRgba(MapStyle::Label));
    const double ascent = QFontMetricsF(painter.font()).ascent();
    constexpr double Margin = 3.0;
    painter.save();
    painter.setClipRect(visible); // The labels too: none above or below the world when it does not fill the widget

    // The meridians, labelled at the top.
    for(auto k = static_cast<qint64>(std::ceil(northWest.x() / step)); k * step <= southEast.x(); ++k)
    {
        const double longitude = static_cast<double>(k) * step;
        const double x = pixelCenter(m_view.toPixels({longitude, 0.0}, viewport).x());
        painter.setPen(line);
        painter.drawLine(QPointF(x, visible.top()), QPointF(x, visible.bottom()));
        painter.setPen(label);
        painter.drawText(QPointF(x + Margin, visible.top() + Margin + ascent), longitudeLabel(longitude));
    }
    // The parallels, labelled on the left.
    for(auto k = static_cast<qint64>(std::ceil(southEast.y() / step)); k * step <= northWest.y(); ++k)
    {
        const double latitude = static_cast<double>(k) * step;
        const double y = pixelCenter(m_view.toPixels({0.0, latitude}, viewport).y());
        painter.setPen(line);
        painter.drawLine(QPointF(visible.left(), y), QPointF(visible.right(), y));
        painter.setPen(label);
        painter.drawText(QPointF(visible.left() + Margin, y - Margin), latitudeLabel(latitude));
    }
    painter.restore();
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }
    m_dragOrigin = event->position();
    setCursor(Qt::ClosedHandCursor);
}

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(!m_dragOrigin)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }
    MapView view = m_view;
    view.pan(event->position() - *m_dragOrigin, size());
    m_dragOrigin = event->position();
    setView(view);
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() != Qt::LeftButton || !m_dragOrigin)
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    m_dragOrigin.reset();
    setCursor(Qt::OpenHandCursor);
}

void MapWidget::wheelEvent(QWheelEvent* event)
{
    const double notches = event->angleDelta().y() / 120.0; // 120 a notch of most mice
    if(notches == 0.0)
    {
        event->ignore();
        return;
    }
    MapView view = m_view;
    view.zoomAt(event->position(), std::pow(2.0, notches / 2.0), size()); // Twice as close every two notches
    setView(view);
}

void MapWidget::setView(const MapView& view)
{
    const MapView previous = std::exchange(m_view, view);
    const bool longitude = m_view.center().x() != previous.center().x();
    const bool latitude = m_view.center().y() != previous.center().y();
    const bool zoom = m_view.zoom() != previous.zoom();
    if(!longitude && !latitude && !zoom)
    {
        return;
    }
    update();
    if(longitude)
    {
        emit longitudeChanged(m_view.center().x());
    }
    if(latitude)
    {
        emit latitudeChanged(m_view.center().y());
    }
    if(zoom)
    {
        emit zoomChanged(m_view.zoom());
    }
}
