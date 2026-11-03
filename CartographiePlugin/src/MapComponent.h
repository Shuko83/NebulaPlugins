#pragma once

#include "MapWidget.h"
#include "Nebula/Component/Component.h"

// A basic 2D map of the world, in a MapWidget it creates and provides as the interface "map", a QWidget, for another
// component to show, the "centralWidget" of a MainWindowComponent say. Its properties are the view of the map: its
// center, in degrees, and its zoom, 1 showing the whole world; they follow the mouse, which drags and zooms the map. It
// needs a QApplication.
class MapComponent : public Nebula::Component
{
    Q_OBJECT
    Q_CLASSINFO("description", "A basic 2D map of the world, provided as a widget")
    Q_CLASSINFO("category", "Cartographie")
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

  public:
    MapComponent();

    // Of the center of the map, from -180 to 180 degrees.
    [[nodiscard]] double longitude() const;
    void setLongitude(double longitude);
    // Of the center of the map, from -90 to 90 degrees.
    [[nodiscard]] double latitude() const;
    void setLatitude(double latitude);
    // From 1, the whole world, to 64.
    [[nodiscard]] double zoom() const;
    void setZoom(double zoom);

  signals:
    void longitudeChanged(double longitude);
    void latitudeChanged(double latitude);
    void zoomChanged(double zoom);

  private:
    MapWidget m_map;
};
