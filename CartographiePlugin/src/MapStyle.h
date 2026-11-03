#pragma once

#include <QRgb>

// The colors MapWidget paints the map with.
namespace MapStyle
{

constexpr QRgb Outside = 0xffdcdcdc;   // Around the world, when it does not fill the widget
constexpr QRgb Sea = 0xffaad3df;
constexpr QRgb Land = 0xfff2efe9;
constexpr QRgb Coastline = 0xff9a9a90; // And the edge of the world
constexpr QRgb Graticule = 0x50506878; // Meridians and parallels, half transparent
constexpr QRgb Label = 0xff4a5560;     // Of the meridians and parallels

} // namespace MapStyle
