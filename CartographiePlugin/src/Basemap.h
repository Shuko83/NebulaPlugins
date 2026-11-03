#pragma once

#include <QByteArray>
#include <QPainterPath>
#include <QString>

// The land of a basemap, read from GeoJSON: the Polygon and MultiPolygon geometries of its features, or of the geometry
// it is, each polygon an outer ring and its holes; the other geometries, points and lines, are not land and are
// skipped. Its path is in degrees, x being the longitude and y the latitude, north upwards (see MapView). A basemap that
// cannot be read has no land, and tells why: "feature 12: a ring has fewer than 3 positions".
class Basemap
{
  public:
    [[nodiscard]] static Basemap fromGeoJson(const QByteArray& json);
    // The land of the world, Natural Earth at 1:110m, compiled into the plugin through basemap.qrc. Read once; an
    // error is written to the log, category "cartographie.basemap".
    [[nodiscard]] static const Basemap& world();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] QString errorString() const;

    // Filled with Qt::OddEvenFill: a hole is drawn as the sea.
    [[nodiscard]] QPainterPath land() const;
    [[nodiscard]] int polygonCount() const;

  private:
    QPainterPath m_land;
    int m_polygonCount = 0;
    QString m_error;
};
