#include <QColor>
#include <QDebug>
#include <QImage>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>
#include <memory>

#include "MapStyle.h"
#include "Nebula/Application/Universe.h"
#include "Nebula/Component/Component.h"

using namespace Qt::StringLiterals;

namespace
{

const QString PluginId = QStringLiteral(CARTOGRAPHIE_PLUGIN_ID);
const QString MapComponentId = PluginId + u".MapComponent"_s;

// A universe in which CartographiePlugin is loaded from its file.
std::unique_ptr<Nebula::Universe> universeWithPlugin()
{
    auto universe = std::make_unique<Nebula::Universe>();
    universe->plugins().scan(QStringLiteral(CARTOGRAPHIE_PLUGIN_DIR));
    if(!universe->plugins().load(PluginId))
    {
        qWarning().noquote() << universe->plugins().errorString();
    }
    return universe;
}

// The color of a pixel of what map draws at that size.
QColor pixelOf(QWidget& map, QSize size, QPoint pixel)
{
    map.resize(size);
    return QColor(map.grab().toImage().pixel(pixel));
}

} // namespace

// CartographiePlugin loaded from its file, as an application loads it: its MapComponent provides a map of the world as
// a widget, drawn here into an image without being shown.
class TestCartographiePlugin : public QObject
{
    Q_OBJECT

  private slots:
    void providesTheMap();
    void propertiesAreTheView();
    void drawsTheLandOnTheSea();
};

void TestCartographiePlugin::providesTheMap()
{
    const std::unique_ptr<Nebula::Universe> universe = universeWithPlugin();
    Nebula::Component* component = universe->instantiate(MapComponentId);
    QVERIFY(component != nullptr);
    QVERIFY(component->providedInterface<QWidget>(u"map"_s) != nullptr); // Available from the start
    QVERIFY(component->unboundMandatoryInterfaces().isEmpty());

    QVERIFY2(universe->start(), qPrintable(universe->errorString()));
}

void TestCartographiePlugin::propertiesAreTheView()
{
    const std::unique_ptr<Nebula::Universe> universe = universeWithPlugin();
    Nebula::Component* component = universe->instantiate(MapComponentId);
    QVERIFY(component != nullptr);
    QCOMPARE(component->property("longitude").toDouble(), 0.0);
    QCOMPARE(component->property("latitude").toDouble(), 0.0);
    QCOMPARE(component->property("zoom").toDouble(), 1.0); // The whole world

    QSignalSpy longitudeChanged(component, SIGNAL(longitudeChanged(double)));
    QVERIFY(component->setProperty("longitude", 2.35));
    QCOMPARE(component->property("longitude").toDouble(), 2.35);
    QCOMPARE(longitudeChanged.count(), 1);
    QVERIFY(component->setProperty("longitude", 2.35));
    QCOMPARE(longitudeChanged.count(), 1); // Unchanged

    // Brought back to the limits.
    QVERIFY(component->setProperty("latitude", 120.0));
    QCOMPARE(component->property("latitude").toDouble(), 90.0);
    QVERIFY(component->setProperty("zoom", 1000.0));
    QCOMPARE(component->property("zoom").toDouble(), 64.0);
}

void TestCartographiePlugin::drawsTheLandOnTheSea()
{
    const std::unique_ptr<Nebula::Universe> universe = universeWithPlugin();
    Nebula::Component* component = universe->instantiate(MapComponentId);
    QVERIFY(component != nullptr);
    QWidget* map = component->providedInterface<QWidget>(u"map"_s);
    QVERIFY(map != nullptr);

    // The whole world, two pixels a degree: (longitude, latitude) at (360 + 2 longitude, 180 - 2 latitude).
    const QSize world(720, 360);
    QCOMPARE(pixelOf(*map, world, {390, 134}), QColor::fromRgba(MapStyle::Land)); // 15°E 23°N, the Sahara
    QCOMPARE(pixelOf(*map, world, {310, 220}), QColor::fromRgba(MapStyle::Sea));  // 25°W 20°S, the South Atlantic
    // Taller than the world: room above and below it.
    QCOMPARE(pixelOf(*map, {720, 480}, {360, 30}), QColor::fromRgba(MapStyle::Outside));

    // Zoomed on Europe, 16 pixels a degree, 10°E 50°N in the middle.
    QVERIFY(component->setProperty("longitude", 10.0));
    QVERIFY(component->setProperty("latitude", 50.0));
    QVERIFY(component->setProperty("zoom", 8.0));
    QCOMPARE(pixelOf(*map, world, {237, 198}), QColor::fromRgba(MapStyle::Land)); // 2.35°E 48.85°N, Paris
    QCOMPARE(pixelOf(*map, world, {104, 244}), QColor::fromRgba(MapStyle::Sea));  // 6°W 46°N, the Bay of Biscay
}

QTEST_MAIN(TestCartographiePlugin)

#include "TestCartographiePlugin.moc"
