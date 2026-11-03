# The classes of the plugin are not in a library: their tests compile their sources.
set(_cartographie_sources "${PROJECT_SOURCE_DIR}/CartographiePlugin/src")

# With the land compiled into the plugin, through the same basemap.qrc.
set(CMAKE_AUTORCC ON)
nebula_plugins_add_test(TestBasemap
    SOURCES "${_cartographie_sources}/Basemap.cpp" "${PROJECT_SOURCE_DIR}/CartographiePlugin/data/basemap.qrc"
    LINK_LIBRARIES Qt6::Gui
    INCLUDE_DIRECTORIES "${_cartographie_sources}"
)
nebula_plugins_add_test(TestMapView
    SOURCES "${_cartographie_sources}/MapView.cpp"
    LINK_LIBRARIES Qt6::Gui
    INCLUDE_DIRECTORIES "${_cartographie_sources}"
)

# The plugin loaded from its file, as an application loads it, its map drawn into an image: no window is shown.
nebula_plugins_add_test(TestCartographiePlugin
    LINK_LIBRARIES Nebula::Core Qt6::Widgets
    INCLUDE_DIRECTORIES "${_cartographie_sources}"
)
target_compile_definitions(TestCartographiePlugin PRIVATE
    "CARTOGRAPHIE_PLUGIN_DIR=\"$<TARGET_FILE_DIR:CartographiePlugin>\""
    "CARTOGRAPHIE_PLUGIN_ID=\"${cmu_organization}.${PROJECT_NAME}.CartographiePlugin\"")
add_dependencies(TestCartographiePlugin CartographiePlugin)
