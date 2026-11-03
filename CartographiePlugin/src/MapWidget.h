#pragma once

#include <QPainterPath>
#include <QPointF>
#include <QWidget>
#include <optional>

#include "MapView.h"

class QPainter;

// A basic 2D map of the world: the land of Basemap::world() on the sea, in plate carrée, under its meridians and
// parallels, labelled at its edges, their spacing following the zoom. Dragging it with the left button moves it, its
// wheel zooms where the mouse points. Its view is its center, a longitude and a latitude in degrees, and its zoom, 1
// showing the whole world (see MapView).
class MapWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit MapWidget(QWidget* parent = nullptr);

    [[nodiscard]] double longitude() const;
    void setLongitude(double longitude);
    [[nodiscard]] double latitude() const;
    void setLatitude(double latitude);
    [[nodiscard]] double zoom() const;
    void setZoom(double zoom);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

  signals:
    // As its properties are set, and as the mouse moves or zooms it.
    void longitudeChanged(double longitude);
    void latitudeChanged(double latitude);
    void zoomChanged(double zoom);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  private:
    // Shows view, telling what changed.
    void setView(const MapView& view);
    void paintGraticule(QPainter& painter) const;

    MapView m_view;
    QPainterPath m_land;                 // In degrees
    std::optional<QPointF> m_dragOrigin; // Where the mouse was, while it drags the map
};
