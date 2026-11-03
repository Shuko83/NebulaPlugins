#include "MapComponent.h"

#include "CartographiePlugin.h"

NEBULA_REGISTER_COMPONENT(CartographiePlugin, MapComponent);

MapComponent::MapComponent()
{
    connect(&m_map, &MapWidget::longitudeChanged, this, &MapComponent::longitudeChanged);
    connect(&m_map, &MapWidget::latitudeChanged, this, &MapComponent::latitudeChanged);
    connect(&m_map, &MapWidget::zoomChanged, this, &MapComponent::zoomChanged);
    // Available at once: the map is drawn as soon as it is shown.
    provideInterface<QWidget>(QStringLiteral("map"), &m_map);
}

double MapComponent::longitude() const
{
    return m_map.longitude();
}

void MapComponent::setLongitude(double longitude)
{
    m_map.setLongitude(longitude);
}

double MapComponent::latitude() const
{
    return m_map.latitude();
}

void MapComponent::setLatitude(double latitude)
{
    m_map.setLatitude(latitude);
}

double MapComponent::zoom() const
{
    return m_map.zoom();
}

void MapComponent::setZoom(double zoom)
{
    m_map.setZoom(zoom);
}
