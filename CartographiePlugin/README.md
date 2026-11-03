# CartographiePlugin

Un fond de carte 2D du monde, volontairement très simple : les terres sur la mer, en projection plate carrée, avec les méridiens et les parallèles. `MapComponent` le fournit comme widget, à placer dans une fenêtre.

## MapComponent

`MapComponent` crée un [`MapWidget`](src/MapWidget.h) et le fournit sous l'interface `map`, un `QWidget`. Pour l'afficher, lier `map` au `centralWidget` d'un `MainWindowComponent` (WidgetsPlugin de Nebula), ou à tout composant qui consomme un `QWidget`. Il ne consomme rien.

| Propriété   | Valeur                                              | Par défaut |
| ----------- | --------------------------------------------------- | ---------- |
| `longitude` | Longitude du centre de la carte, de -180 à 180°     | `0`        |
| `latitude`  | Latitude du centre de la carte, de -90 à 90°        | `0`        |
| `zoom`      | De 1, le monde entier dans le widget, à 64          | `1`        |

Une valeur hors limites est ramenée à la limite la plus proche : une latitude de 120 devient 90.

À la souris, glisser avec le bouton gauche déplace la carte, et la molette zoome sous le curseur (×2 tous les deux crans). Les propriétés suivent : un fichier de réglages qui les tient garde donc la vue de l'utilisateur d'un lancement à l'autre.

## Le fond de carte

- **Projection** : plate carrée (équirectangulaire). Un degré de longitude et un degré de latitude prennent autant de pixels. Au zoom 1, le monde entier tient dans le widget et y est centré ; ce qui reste autour est gris. Le calcul est fait par [`MapView`](src/MapView.h).
- **Terres** : [Natural Earth](https://www.naturalearthdata.com/), couche *Land* au 1:110m, du domaine public. Le fichier [data/ne_110m_land.geojson](data/ne_110m_land.geojson) est celui du dépôt [nvkelso/natural-earth-vector](https://github.com/nvkelso/natural-earth-vector/blob/master/geojson/ne_110m_land.geojson), inchangé (dernière modification en amont : commit `693f114`). Il est **embarqué** dans `CartographiePlugin.nbdl` par la ressource Qt [data/basemap.qrc](data/basemap.qrc), et lu une seule fois par [`Basemap`](src/Basemap.h).
- **Graticule** : le pas des méridiens et des parallèles suit le zoom (30°, 15°, 10°, 5°, 2°, 1°, 0,5°, 0,25°, 0,1°), pour garder au moins 60 pixels entre deux lignes. Ils sont étiquetés en haut et à gauche : `30°E`, `45°N`.
- **Couleurs** : dans [src/MapStyle.h](src/MapStyle.h).

Si le fichier embarqué ne peut pas être lu, la carte n'a que la mer et le graticule, et l'erreur est écrite dans le journal, catégorie `cartographie.basemap`.

## Changer de fond

`Basemap::fromGeoJson()` lit tout GeoJSON en degrés (WGS84) : une `FeatureCollection`, une `Feature` ou une géométrie. Les `Polygon` et les `MultiPolygon` sont les terres, leurs trous la mer ; les points et les lignes sont ignorés. Un fichier mal formé, ou dont une position sort du monde (des mètres de Web Mercator, par exemple), n'est pas lu du tout, et l'erreur dit pourquoi : `feature 12: a ring has fewer than 3 positions`.

Pour un fond plus détaillé, remplacer le fichier par la couche *Land* au 1:50m de Natural Earth, plus précise et plus lourde, en gardant son nom ou en changeant celui de `basemap.qrc` et de `Basemap.cpp`, puis compiler et lancer les tests (`ctest -C Release`) : `TestBasemap` compte les 127 polygones du 1:110m, à mettre à jour.

## Limites

- Au 1:110m, les côtes sont grossières au-delà d'un zoom de 8 environ, et les petites îles et les lacs (la Caspienne, par exemple) n'y sont pas.
- Pas de tuiles ni de réseau : le fond est le même hors ligne, à toutes les échelles.
- La carte s'arrête à l'antiméridien : elle ne se répète pas vers l'est ou l'ouest.
- Ni marqueurs ni couches pour l'instant : seulement le fond.
