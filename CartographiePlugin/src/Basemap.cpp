#include "Basemap.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLoggingCategory>

namespace
{

Q_LOGGING_CATEGORY(lcBasemap, "cartographie.basemap")

// The land of the world, compiled into the plugin from data/.
constexpr QLatin1StringView WorldFile(":/CartographiePlugin/data/ne_110m_land.geojson");

// Reads the polygons of GeoJSON objects into a path, up to its first error.
class GeoJsonReader
{
  public:
    // A FeatureCollection, a Feature or a geometry.
    bool readObject(const QJsonObject& object, const QString& where)
    {
        const QString type = object.value(QStringLiteral("type")).toString();
        if(type == QStringLiteral("FeatureCollection"))
        {
            const QJsonValue features = object.value(QStringLiteral("features"));
            if(!features.isArray())
            {
                return fail(where, QStringLiteral("a FeatureCollection has no array of features"));
            }
            const QJsonArray array = features.toArray();
            for(qsizetype i = 0; i < array.size(); ++i)
            {
                const QString feature = QStringLiteral("feature %1").arg(i + 1);
                if(!array.at(i).isObject())
                {
                    return fail(feature, QStringLiteral("not an object"));
                }
                if(!readObject(array.at(i).toObject(), feature))
                {
                    return false;
                }
            }
            return true;
        }
        if(type == QStringLiteral("Feature"))
        {
            const QJsonValue geometry = object.value(QStringLiteral("geometry"));
            if(geometry.isNull())
            {
                return true; // A feature without geometry, which GeoJSON allows
            }
            if(!geometry.isObject())
            {
                return fail(where, QStringLiteral("its geometry is not an object"));
            }
            return readGeometry(geometry.toObject(), where);
        }
        return readGeometry(object, where);
    }

    QPainterPath path;
    int polygonCount = 0;
    QString error;

  private:
    bool readGeometry(const QJsonObject& geometry, const QString& where)
    {
        const QString type = geometry.value(QStringLiteral("type")).toString();
        if(type == QStringLiteral("Polygon"))
        {
            return readPolygon(geometry.value(QStringLiteral("coordinates")), where);
        }
        if(type == QStringLiteral("MultiPolygon"))
        {
            const QJsonValue polygons = geometry.value(QStringLiteral("coordinates"));
            if(!polygons.isArray())
            {
                return fail(where, QStringLiteral("a MultiPolygon has no array of coordinates"));
            }
            for(const QJsonValue& polygon : polygons.toArray())
            {
                if(!readPolygon(polygon, where))
                {
                    return false;
                }
            }
            return true;
        }
        if(type == QStringLiteral("GeometryCollection"))
        {
            const QJsonValue geometries = geometry.value(QStringLiteral("geometries"));
            if(!geometries.isArray())
            {
                return fail(where, QStringLiteral("a GeometryCollection has no array of geometries"));
            }
            for(const QJsonValue& member : geometries.toArray())
            {
                if(!member.isObject())
                {
                    return fail(where, QStringLiteral("a geometry of a GeometryCollection is not an object"));
                }
                if(!readGeometry(member.toObject(), where))
                {
                    return false;
                }
            }
            return true;
        }
        if(type == QStringLiteral("Point") || type == QStringLiteral("MultiPoint") ||
           type == QStringLiteral("LineString") || type == QStringLiteral("MultiLineString"))
        {
            return true; // Not land
        }
        return fail(where, QStringLiteral("unknown GeoJSON type \"%1\"").arg(type));
    }

    // An outer ring, then its holes.
    bool readPolygon(const QJsonValue& rings, const QString& where)
    {
        if(!rings.isArray() || rings.toArray().isEmpty())
        {
            return fail(where, QStringLiteral("a Polygon has no array of rings"));
        }
        for(const QJsonValue& ring : rings.toArray())
        {
            if(!readRing(ring, where))
            {
                return false;
            }
        }
        ++polygonCount;
        return true;
    }

    // Positions [longitude, latitude], in degrees. GeoJSON repeats the first one last; the path closes the ring
    // whether it does or not, so that three positions are enough.
    bool readRing(const QJsonValue& ring, const QString& where)
    {
        const QJsonArray positions = ring.toArray();
        if(positions.size() < 3)
        {
            return fail(where, QStringLiteral("a ring has fewer than 3 positions"));
        }
        for(qsizetype i = 0; i < positions.size(); ++i)
        {
            const QJsonArray position = positions.at(i).toArray();
            if(position.size() < 2 || !position.at(0).isDouble() || !position.at(1).isDouble())
            {
                return fail(where, QStringLiteral("a position is not [longitude, latitude]"));
            }
            const QPointF point(position.at(0).toDouble(), position.at(1).toDouble());
            if(point.x() < -180.0 || point.x() > 180.0 || point.y() < -90.0 || point.y() > 90.0)
            {
                return fail(where, QStringLiteral("position [%1, %2] out of the world: not in degrees")
                                       .arg(point.x())
                                       .arg(point.y()));
            }
            if(i == 0)
            {
                path.moveTo(point);
            }
            else
            {
                path.lineTo(point);
            }
        }
        path.closeSubpath();
        return true;
    }

    bool fail(const QString& where, const QString& message)
    {
        error = where.isEmpty() ? message : where + QStringLiteral(": ") + message;
        return false;
    }
};

} // namespace

Basemap Basemap::fromGeoJson(const QByteArray& json)
{
    Basemap basemap;
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if(parseError.error != QJsonParseError::NoError)
    {
        basemap.m_error =
            QStringLiteral("not JSON: %1 at offset %2").arg(parseError.errorString()).arg(parseError.offset);
        return basemap;
    }
    if(!document.isObject())
    {
        basemap.m_error = QStringLiteral("not a GeoJSON object");
        return basemap;
    }

    GeoJsonReader reader;
    reader.path.setFillRule(Qt::OddEvenFill);
    if(!reader.readObject(document.object(), {}))
    {
        basemap.m_error = reader.error; // No land at all rather than part of it
        return basemap;
    }
    basemap.m_land = reader.path;
    basemap.m_polygonCount = reader.polygonCount;
    return basemap;
}

const Basemap& Basemap::world()
{
    static const Basemap world = [] {
        QFile file(WorldFile);
        Basemap basemap;
        if(!file.open(QIODevice::ReadOnly))
        {
            basemap.m_error = QStringLiteral("cannot read %1: %2").arg(WorldFile, file.errorString());
        }
        else
        {
            basemap = fromGeoJson(file.readAll());
        }
        if(!basemap.isValid())
        {
            qCWarning(lcBasemap).noquote() << "The map has no land:" << basemap.errorString();
        }
        return basemap;
    }();
    return world;
}

bool Basemap::isValid() const
{
    return m_error.isEmpty();
}

QString Basemap::errorString() const
{
    return m_error;
}

QPainterPath Basemap::land() const
{
    return m_land;
}

int Basemap::polygonCount() const
{
    return m_polygonCount;
}
