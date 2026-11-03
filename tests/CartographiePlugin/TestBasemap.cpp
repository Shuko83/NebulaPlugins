#include <QTest>

#include "Basemap.h"

using namespace Qt::StringLiterals;

// The land of the GeoJSON files: their polygons, in degrees, x being the longitude and y the latitude.
class TestBasemap : public QObject
{
    Q_OBJECT

  private slots:
    void worldIsEmbedded();
    void polygon();
    void featuresAndMultiPolygons();
    void holeIsSea();
    void otherGeometriesAreSkipped();
    void errors_data();
    void errors();
};

void TestBasemap::worldIsEmbedded()
{
    const Basemap& world = Basemap::world();
    QVERIFY2(world.isValid(), qPrintable(world.errorString()));
    QCOMPARE(world.polygonCount(), 127); // Those of ne_110m_land.geojson

    const QRectF bounds = world.land().boundingRect();
    QCOMPARE(bounds.left(), -180.0);
    QCOMPARE(bounds.right(), 180.0);
    QCOMPARE(bounds.top(), -90.0); // The south of Antarctica: y is the latitude
    QVERIFY(bounds.bottom() > 80.0 && bounds.bottom() < 90.0);

    QVERIFY(world.land().contains(QPointF(15.0, 23.0)));    // The Sahara
    QVERIFY(world.land().contains(QPointF(2.35, 48.85)));   // Paris
    QVERIFY(!world.land().contains(QPointF(-25.0, -20.0))); // The South Atlantic
    QVERIFY(!world.land().contains(QPointF(-150.0, 0.0)));  // The Pacific
}

void TestBasemap::polygon()
{
    const Basemap basemap =
        Basemap::fromGeoJson(R"({"type": "Polygon", "coordinates": [[[0, 0], [10, 0], [10, 10], [0, 10], [0, 0]]]})"_ba);
    QVERIFY2(basemap.isValid(), qPrintable(basemap.errorString()));
    QCOMPARE(basemap.polygonCount(), 1);
    QVERIFY(basemap.land().contains(QPointF(5.0, 5.0)));
    QVERIFY(!basemap.land().contains(QPointF(15.0, 5.0)));
}

void TestBasemap::featuresAndMultiPolygons()
{
    const Basemap basemap = Basemap::fromGeoJson(R"({"type": "FeatureCollection", "features": [
        {"type": "Feature", "properties": {}, "geometry": {"type": "MultiPolygon", "coordinates": [
            [[[0, 0], [10, 0], [10, 10], [0, 0]]],
            [[[20, 0], [30, 0], [30, 10], [20, 0]]]]}},
        {"type": "Feature", "properties": {}, "geometry": null},
        {"type": "Feature", "properties": {}, "geometry": {"type": "Polygon", "coordinates": [
            [[-10, -10], [-5, -10], [-5, -5]]]}}]})"_ba);
    QVERIFY2(basemap.isValid(), qPrintable(basemap.errorString()));
    QCOMPARE(basemap.polygonCount(), 3); // Three positions are enough: the path closes the ring
    QVERIFY(basemap.land().contains(QPointF(25.0, 2.0)));
    QVERIFY(basemap.land().contains(QPointF(-6.0, -9.0)));
}

void TestBasemap::holeIsSea()
{
    const Basemap basemap = Basemap::fromGeoJson(R"({"type": "Polygon", "coordinates": [
        [[0, 0], [10, 0], [10, 10], [0, 10], [0, 0]],
        [[4, 4], [6, 4], [6, 6], [4, 6], [4, 4]]]})"_ba);
    QVERIFY2(basemap.isValid(), qPrintable(basemap.errorString()));
    QCOMPARE(basemap.polygonCount(), 1);
    QVERIFY(basemap.land().contains(QPointF(2.0, 2.0)));
    QVERIFY(!basemap.land().contains(QPointF(5.0, 5.0)));
}

void TestBasemap::otherGeometriesAreSkipped()
{
    const Basemap basemap = Basemap::fromGeoJson(R"({"type": "GeometryCollection", "geometries": [
        {"type": "Point", "coordinates": [1, 1]},
        {"type": "LineString", "coordinates": [[0, 0], [1, 1]]},
        {"type": "Polygon", "coordinates": [[[0, 0], [10, 0], [10, 10], [0, 0]]]}]})"_ba);
    QVERIFY2(basemap.isValid(), qPrintable(basemap.errorString()));
    QCOMPARE(basemap.polygonCount(), 1);
}

void TestBasemap::errors_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<QString>("error");

    QTest::newRow("not JSON") << R"({"type": )"_ba << u"not JSON: "_s;
    QTest::newRow("not an object") << R"([1, 2])"_ba << u"not a GeoJSON object"_s;
    QTest::newRow("unknown type") << R"({"type": "Circle"})"_ba << u"unknown GeoJSON type \"Circle\""_s;
    QTest::newRow("no features") << R"({"type": "FeatureCollection"})"_ba
                                 << u"a FeatureCollection has no array of features"_s;
    QTest::newRow("no rings") << R"({"type": "Polygon", "coordinates": []})"_ba << u"a Polygon has no array of rings"_s;
    // The first feature is land, the second is wrong: none is kept.
    QTest::newRow("short ring") << R"({"type": "FeatureCollection", "features": [
        {"type": "Feature", "geometry": {"type": "Polygon", "coordinates": [[[0, 0], [10, 0], [10, 10]]]}},
        {"type": "Feature", "geometry": {"type": "Polygon", "coordinates": [[[0, 0], [10, 0]]]}}]})"_ba
                                << u"feature 2: a ring has fewer than 3 positions"_s;
    QTest::newRow("not a position") << R"({"type": "Polygon", "coordinates": [[["a", 0], [10, 0], [10, 10]]]})"_ba
                                    << u"a position is not [longitude, latitude]"_s;
    // In metres, as Web Mercator has them, rather than in degrees.
    QTest::newRow("projected") << R"({"type": "Polygon", "coordinates": [[[0, 0], [1000000, 0], [0, 5000000]]]})"_ba
                               << u"position [1e+06, 0] out of the world: not in degrees"_s;
}

void TestBasemap::errors()
{
    QFETCH(QByteArray, json);
    QFETCH(QString, error);

    const Basemap basemap = Basemap::fromGeoJson(json);
    QVERIFY(!basemap.isValid());
    QVERIFY2(basemap.errorString().startsWith(error), qPrintable(basemap.errorString()));
    QVERIFY(basemap.land().isEmpty());
    QCOMPARE(basemap.polygonCount(), 0);
}

QTEST_GUILESS_MAIN(TestBasemap)

#include "TestBasemap.moc"
