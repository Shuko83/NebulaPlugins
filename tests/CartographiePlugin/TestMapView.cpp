#include <QTest>
#include <cmath>
#include <limits>

#include "MapView.h"

namespace
{

const QSizeF Viewport(720.0, 360.0); // Two pixels a degree at zoom 1
const QPointF Paris(2.35, 48.85);

} // namespace

// What a map shows of the world, in plate carrée, and how the mouse moves it.
class TestMapView : public QObject
{
    Q_OBJECT

  private slots:
    void wholeWorldAtZoomOne();
    void fitsTheViewport();
    void staysOnTheWorld();
    void toGeographicIsTheInverse();
    void panMovesTheMapWithTheMouse();
    void zoomAtKeepsThePointUnderTheMouse();
    void emptyViewport();
};

void TestMapView::wholeWorldAtZoomOne()
{
    const MapView view;
    QCOMPARE(view.center(), QPointF(0.0, 0.0));
    QCOMPARE(view.zoom(), 1.0);
    QCOMPARE(view.pixelsPerDegree(Viewport), 2.0);

    QCOMPARE(view.toPixels({0.0, 0.0}, Viewport), QPointF(360.0, 180.0));
    QCOMPARE(view.toPixels({-180.0, 90.0}, Viewport), QPointF(0.0, 0.0)); // North upwards
    QCOMPARE(view.toPixels({180.0, -90.0}, Viewport), QPointF(720.0, 360.0));
}

void TestMapView::fitsTheViewport()
{
    const MapView view;
    QCOMPARE(view.pixelsPerDegree({720.0, 480.0}), 2.0); // The width fits, the height has room around it
    QCOMPARE(view.toPixels({0.0, 90.0}, {720.0, 480.0}), QPointF(360.0, 60.0));
    QCOMPARE(view.pixelsPerDegree({360.0, 360.0}), 1.0);

    const MapView zoomed(Paris, 8.0);
    QCOMPARE(zoomed.pixelsPerDegree(Viewport), 16.0);
    QCOMPARE(zoomed.toPixels(Paris, Viewport), QPointF(360.0, 180.0));
}

void TestMapView::staysOnTheWorld()
{
    MapView view({200.0, -100.0}, 1000.0);
    QCOMPARE(view.center(), QPointF(180.0, -90.0));
    QCOMPARE(view.zoom(), MapView::MaxZoom);

    view.setZoom(0.5);
    QCOMPARE(view.zoom(), MapView::MinZoom);

    // Not a number: ignored.
    view.setCenter(Paris);
    view.setCenter({std::numeric_limits<double>::quiet_NaN(), 0.0});
    view.setZoom(std::numeric_limits<double>::infinity());
    QCOMPARE(view.center(), Paris);
    QCOMPARE(view.zoom(), MapView::MinZoom);
}

void TestMapView::toGeographicIsTheInverse()
{
    const MapView view(Paris, 8.0);
    QCOMPARE(view.toGeographic({360.0, 180.0}, Viewport), Paris);
    const QPointF pixels(100.0, 300.0);
    QCOMPARE(view.toPixels(view.toGeographic(pixels, Viewport), Viewport), pixels);
}

void TestMapView::panMovesTheMapWithTheMouse()
{
    MapView view;
    // Dragged 20 pixels right and 10 up: what was west and south of the center comes to it.
    view.pan({20.0, -10.0}, Viewport);
    QCOMPARE(view.center(), QPointF(-10.0, -5.0));

    // No further than the edge of the world.
    view.pan({-2000.0, 0.0}, Viewport);
    QCOMPARE(view.center(), QPointF(180.0, -5.0));
}

void TestMapView::zoomAtKeepsThePointUnderTheMouse()
{
    MapView view;
    const QPointF pixels = view.toPixels(Paris, Viewport);
    view.zoomAt(pixels, 2.0, Viewport);
    QCOMPARE(view.zoom(), 2.0);
    QCOMPARE(view.toGeographic(pixels, Viewport), Paris);

    view.zoomAt(pixels, 1000.0, Viewport);
    QCOMPARE(view.zoom(), MapView::MaxZoom);
    QCOMPARE(view.toGeographic(pixels, Viewport), Paris);

    view.zoomAt(pixels, 0.001, Viewport);
    QCOMPARE(view.zoom(), MapView::MinZoom);
}

void TestMapView::emptyViewport()
{
    const MapView view(Paris, 4.0);
    QCOMPARE(view.pixelsPerDegree({}), 4.0);
    const QPointF geographic = view.toGeographic({}, {});
    QVERIFY(std::isfinite(geographic.x()) && std::isfinite(geographic.y()));
}

QTEST_GUILESS_MAIN(TestMapView)

#include "TestMapView.moc"
