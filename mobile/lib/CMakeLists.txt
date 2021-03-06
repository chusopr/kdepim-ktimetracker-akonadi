include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditor-ng
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
)

add_subdirectory( calendar )
add_subdirectory( docs )

set(mobileui_SRCS
    agentstatusmonitor.cpp
    akonadibreadcrumbnavigationfactory.cpp
    breadcrumbnavigation.cpp
    breadcrumbnavigationcontext.cpp
    collectionfetchwatcher.cpp
    declarativewidgetbase.cpp
    declarativeakonadiitem.cpp
    exporthandlerbase.cpp
    favoriteseditor.cpp
    favoritescontroller.cpp
    favoriteslistmodel.cpp
    guistatemanager.cpp
    hierarchyresolver.cpp
    importhandlerbase.cpp
    kcolorcombo.cpp
    kdeclarativeapplication.cpp
    kdeclarativefullscreenview.cpp
    kdeclarativemainview.cpp
    kdeclarativemainview_p.cpp
    kresettingproxymodel.cpp
    listproxy.cpp
    modelselectiontransition.cpp
    orderedchildcollectionsmodel.cpp
    qmllistselectionmodel.cpp
    rangemanager.cpp
    searchmanager.cpp
    statemachinebuilder.cpp
    stylesheetloader.cpp
    threadmodel.cpp
    threadgroupermodel.cpp
    threadselectionmodel.cpp
)

if (KDEQMLPLUGIN_STATIC)
  add_definitions(-DKDEQMLPLUGIN_STATIC)
endif (KDEQMLPLUGIN_STATIC)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_library(kdepimmobileui ${LIBRARY_TYPE} ${mobileui_SRCS})
set_target_properties(kdepimmobileui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(kdepimmobileui
  ${QT_QTDECLARATIVE_LIBRARY}
  ${QT_QTOPENGL_LIBRARY}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDE4_SOLID_LIBRARY}
  akonadi_next
  kdepim
  calendarsupport
)

if (KDEQMLPLUGIN_STATIC)
  target_link_libraries(kdepimmobileui kdeqmlplugin)
endif (KDEQMLPLUGIN_STATIC)

if (X11_FOUND)
  target_link_libraries(kdepimmobileui ${X11_LIBRARIES})
endif()

#FIXME: Dont use fixed Paths here, we know that under wince it should be installed to /Programme/Kontact-Mobile
# but would be nicer if it is flexible
if (NOT WINCE)
    set( STYLE_IMAGE_PATH ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui/images )
else (NOT WINCE)
    set( STYLE_IMAGE_PATH /Programme/Kontact-Mobile/${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui/images )
endif(NOT WINCE)
configure_file( stylesheet.css.cmake ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css )

add_subdirectory(images)

install(TARGETS kdepimmobileui ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  qmldir
  Action.qml
  ActionButton.qml
  ActiveActionMenuItemDelegate.qml
  BulkActionScreen.qml
  BulkActionList.qml
  DecoratedFlickable.qml
  DecoratedListView.qml
  FakeAction.qml
  FavoriteManager.qml
  ActionList.qml
  ReorderList.qml
  ReorderListDelegate.qml
  MultipleSelectionComponent.qml
  MultipleSelectionScreen.qml
  ReorderListContainer.qml
  ActionListItem.qml
  ActionMenuContainer.qml
  AgentInstanceList.qml
  AgentInstanceListContainer.qml
  AgentInstanceListDelegate.qml
  AttachmentList.qml
  ApplicationGeneralActions.qml
  Button.qml
  Button2.qml
  AboutDialog.qml
  CompletionSlider.qml
  ItemListView.qml
  ItemListViewDelegate.qml
  MainView.qml
  ScriptAction.qml
  ScriptActionItem.qml
  StartCanvas.qml
  Spinner.qml
  WindowBar.qml
  VerticalSelector.qml
  Clock.qml
  Calendar.qml
  CalendarDay.qml
  AgentStatusIndicator.qml
  SearchDialog.qml
  SearchResultScreen.qml
  ItemEditButton.qml
  Switch.qml
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
  ScreenFunctions.js
  DESTINATION ${DATA_INSTALL_DIR}/mobileui)

add_subdirectory( tests )
