FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_gfx REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(SDL_ttf REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(PhysFS REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)

IF (CMAKE_COMPILER_IS_GNUCXX)
    # Help getting compilation warnings
    SET(CMAKE_CXX_FLAGS "-Wall")
    IF (WIN32)
        # This includes enough debug information to get something useful
        # from Dr. Mingw while keeping binary size down. Almost useless
        # with gdb, though.
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb0 -gstabs2")
    ENDIF()
ENDIF()

SET(FLAGS "-DPACKAGE_VERSION=\\\"${VERSION}\\\"")
SET(FLAGS "${FLAGS} -DPKG_DATADIR=\\\"${PKG_DATADIR}/\\\"")
SET(FLAGS "${FLAGS} -DLOCALEDIR=\\\"${LOCALEDIR}/\\\"")

IF (ENABLE_NLS)
    SET(FLAGS "${FLAGS} -DENABLE_NLS=1")
ENDIF()

IF (CMAKE_BUILD_TYPE)
    STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
    IF((CMAKE_BUILD_TYPE_TOLOWER MATCHES debug) OR
       (CMAKE_BUILD_TYPE_TOLOWER MATCHES relwithdebinfo))
        SET(FLAGS "${FLAGS} -DDEBUG")
    ENDIF()
ENDIF()

IF (WIN32)
    SET(EXTRA_LIBRARIES ws2_32 winmm)
    FIND_PACKAGE(LibIntl REQUIRED)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL SunOS)
    # explicit linking to libintl is required on Solaris
    SET(EXTRA_LIBRARIES intl)
ENDIF()

FIND_PACKAGE(Guichan REQUIRED ${GUICHAN_COMPONENTS})

IF (WITH_OPENGL)
    FIND_PACKAGE(OpenGL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    SET(FLAGS "${FLAGS} -DUSE_OPENGL")
ENDIF (WITH_OPENGL)

IF (USE_X11)
    FIND_PACKAGE(X11 REQUIRED)
    INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
    SET(FLAGS "${FLAGS} -DUSE_X11")
ENDIF (USE_X11)

INCLUDE_DIRECTORIES(
    ${CURRENT_SOURCE_DIR}
    ${SDL_INCLUDE_DIR}
    ${SDLGFX_INCLUDE_DIR}
    ${SDLIMAGE_INCLUDE_DIR}
    ${SDLMIXER_INCLUDE_DIR}
    ${SDLNET_INCLUDE_DIR}
    ${SDLTTF_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${GUICHAN_INCLUDE_DIR}
    )

# Fix some stuff that gets not hidden by mainline modules
MARK_AS_ADVANCED(PHYSFS_INCLUDE_DIR)
MARK_AS_ADVANCED(PHYSFS_LIBRARY)
MARK_AS_ADVANCED(SDLIMAGE_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLIMAGE_LIBRARY)
MARK_AS_ADVANCED(SDLMAIN_LIBRARY)
MARK_AS_ADVANCED(SDLMIXER_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLMIXER_LIBRARY)
MARK_AS_ADVANCED(SDLNET_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLNET_LIBRARY)
MARK_AS_ADVANCED(SDL_INCLUDE_DIR)
MARK_AS_ADVANCED(SDL_LIBRARY)

SET(SRCS
    bindings/curl/downloadwrapper.cpp
    bindings/curl/downloadwrapper.h
    bindings/guichan/graphics.cpp
    bindings/guichan/graphics.h
    bindings/guichan/guichanfwd.h
    bindings/guichan/gui.cpp
    bindings/guichan/gui.h
    bindings/guichan/inputmanager.cpp
    bindings/guichan/inputmanager.h
    bindings/guichan/layout.cpp
    bindings/guichan/layout.h
    bindings/guichan/layouthelper.cpp
    bindings/guichan/layouthelper.h
    bindings/guichan/palette.cpp
    bindings/guichan/palette.h
    bindings/guichan/protectedfocuslistener.cpp
    bindings/guichan/protectedfocuslistener.h
    bindings/guichan/skin.cpp
    bindings/guichan/skin.h
    bindings/guichan/text.cpp
    bindings/guichan/text.h
    bindings/guichan/textmanager.cpp
    bindings/guichan/textmanager.h
    bindings/guichan/textrenderer.h
    bindings/guichan/truetypefont.cpp
    bindings/guichan/truetypefont.h
    bindings/guichan/dialogs/confirmdialog.cpp
    bindings/guichan/dialogs/confirmdialog.h
    bindings/guichan/dialogs/helpdialog.cpp
    bindings/guichan/dialogs/helpdialog.h
    bindings/guichan/dialogs/listdialog.cpp
    bindings/guichan/dialogs/listdialog.h
    bindings/guichan/dialogs/okdialog.cpp
    bindings/guichan/dialogs/okdialog.h
    bindings/guichan/dialogs/setupdialog.cpp
    bindings/guichan/dialogs/setupdialog.h
    bindings/guichan/dialogs/textinputdialog.cpp
    bindings/guichan/dialogs/textinputdialog.h
    bindings/guichan/dialogs/tabs/setup_audio.cpp
    bindings/guichan/dialogs/tabs/setup_audio.h
    bindings/guichan/dialogs/tabs/setup_colors.cpp
    bindings/guichan/dialogs/tabs/setup_colors.h
    bindings/guichan/dialogs/tabs/setup_gui.cpp
    bindings/guichan/dialogs/tabs/setup_gui.h
    bindings/guichan/dialogs/tabs/setup_input.cpp
    bindings/guichan/dialogs/tabs/setup_input.h
    bindings/guichan/handlers/focushandler.cpp
    bindings/guichan/handlers/focushandler.h
    bindings/guichan/handlers/lettertextwraphandler.cpp
    bindings/guichan/handlers/lettertextwraphandler.h
    bindings/guichan/handlers/linkhandler.h
    bindings/guichan/handlers/shortcuthandler.cpp
    bindings/guichan/handlers/shortcuthandler.h
    bindings/guichan/handlers/textwraphandler.h
    bindings/guichan/handlers/wordtextwraphandler.cpp
    bindings/guichan/handlers/wordtextwraphandler.h
    bindings/guichan/models/keylistmodel.h
    bindings/guichan/models/linkmappedlistmodel.cpp
    bindings/guichan/models/linkmappedlistmodel.h
    bindings/guichan/models/mappedlistmodel.h
    bindings/guichan/models/modelistmodel.cpp
    bindings/guichan/models/modelistmodel.h
    bindings/guichan/models/statictablemodel.cpp
    bindings/guichan/models/statictablemodel.h
    bindings/guichan/models/tablemodel.cpp
    bindings/guichan/models/tablemodel.h
    bindings/guichan/opengl/openglgraphics.cpp
    bindings/guichan/opengl/openglgraphics.h
    bindings/guichan/sdl/sdlgraphics.cpp
    bindings/guichan/sdl/sdlgraphics.h
    bindings/guichan/sdl/sdlinput.cpp
    bindings/guichan/sdl/sdlinput.h
    bindings/guichan/widgets/beingbox.cpp
    bindings/guichan/widgets/beingbox.h
    bindings/guichan/widgets/button.cpp
    bindings/guichan/widgets/button.h
    bindings/guichan/widgets/chatinput.cpp
    bindings/guichan/widgets/chatinput.h
    bindings/guichan/widgets/checkbox.cpp
    bindings/guichan/widgets/checkbox.h
    bindings/guichan/widgets/container.cpp
    bindings/guichan/widgets/container.h
    bindings/guichan/widgets/desktop.cpp
    bindings/guichan/widgets/desktop.h
    bindings/guichan/widgets/dropdown.cpp
    bindings/guichan/widgets/dropdown.h
    bindings/guichan/widgets/icon.cpp
    bindings/guichan/widgets/icon.h
    bindings/guichan/widgets/imagebutton.cpp
    bindings/guichan/widgets/imagebutton.h
    bindings/guichan/widgets/inttextfield.h
    bindings/guichan/widgets/inttextfield.cpp
    bindings/guichan/widgets/label.cpp
    bindings/guichan/widgets/label.h
    bindings/guichan/widgets/listbox.cpp
    bindings/guichan/widgets/listbox.h
    bindings/guichan/widgets/mappedlistbox.cpp
    bindings/guichan/widgets/mappedlistbox.h
    bindings/guichan/widgets/passwordfield.cpp
    bindings/guichan/widgets/passwordfield.h
    bindings/guichan/widgets/popup.cpp
    bindings/guichan/widgets/popup.h
    bindings/guichan/widgets/progressbar.cpp
    bindings/guichan/widgets/progressbar.h
    bindings/guichan/widgets/proxywidget.cpp
    bindings/guichan/widgets/proxywidget.h
    bindings/guichan/widgets/radiobutton.cpp
    bindings/guichan/widgets/radiobutton.h
    bindings/guichan/widgets/resizegrip.cpp
    bindings/guichan/widgets/resizegrip.h
    bindings/guichan/widgets/richtextbox.cpp
    bindings/guichan/widgets/richtextbox.h
    bindings/guichan/widgets/scrollarea.cpp
    bindings/guichan/widgets/scrollarea.h
    bindings/guichan/widgets/setuptabcontainer.cpp
    bindings/guichan/widgets/setuptabcontainer.h
    bindings/guichan/widgets/shortcutcontainer.cpp
    bindings/guichan/widgets/shortcutcontainer.h
    bindings/guichan/widgets/shortcutwindow.cpp
    bindings/guichan/widgets/shortcutwindow.h
    bindings/guichan/widgets/slider.cpp
    bindings/guichan/widgets/slider.h
    bindings/guichan/widgets/speechbubble.cpp
    bindings/guichan/widgets/speechbubble.h
    bindings/guichan/widgets/tab.cpp
    bindings/guichan/widgets/tab.h
    bindings/guichan/widgets/tabbedarea.cpp
    bindings/guichan/widgets/tabbedarea.h
    bindings/guichan/widgets/table.cpp
    bindings/guichan/widgets/table.h
    bindings/guichan/widgets/textbox.cpp
    bindings/guichan/widgets/textbox.h
    bindings/guichan/widgets/textfield.cpp
    bindings/guichan/widgets/textfield.h
    bindings/guichan/widgets/textpreview.cpp
    bindings/guichan/widgets/textpreview.h
    bindings/guichan/widgets/tooltip.cpp
    bindings/guichan/widgets/tooltip.h
    bindings/guichan/widgets/window.cpp
    bindings/guichan/widgets/window.h
    bindings/sdl/joystick.cpp
    bindings/sdl/joystick.h
    bindings/sdl/keyboardconfig.cpp
    bindings/sdl/keyboardconfig.h
    bindings/sdl/sound.cpp
    bindings/sdl/sound.h
    bindings/update/downloadupdates.cpp
    bindings/update/downloadupdates.h
    core/configlistener.h
    core/configuration.cpp
    core/configuration.h
    core/log.cpp
    core/log.h
    core/recorder.cpp
    core/recorder.h
    core/resource.cpp
    core/resource.h
    core/resourcemanager.cpp
    core/resourcemanager.h
    core/image/animation.cpp
    core/image/animation.h
    core/image/dye.cpp
    core/image/dye.h
    core/image/image.cpp
    core/image/image.h
    core/image/imageloader.cpp
    core/image/imageloader.h
    core/image/imageset.h
    core/image/imageset.cpp
    core/image/imagewriter.cpp
    core/image/imagewriter.h
    core/image/simpleanimation.cpp
    core/image/simpleanimation.h
    core/image/wallpapermanager.cpp
    core/image/wallpapermanager.h
    core/image/particle/animationparticle.cpp
    core/image/particle/animationparticle.h
    core/image/particle/imageparticle.cpp
    core/image/particle/imageparticle.h
    core/image/particle/particle.cpp
    core/image/particle/particle.h
    core/image/particle/particlecontainer.cpp
    core/image/particle/particlecontainer.h
    core/image/particle/particleemitter.cpp
    core/image/particle/particleemitter.h
    core/image/particle/particleemitterprop.h
    core/image/particle/rotationalparticle.cpp
    core/image/particle/rotationalparticle.h
    core/image/particle/textparticle.cpp
    core/image/particle/textparticle.h
    core/map/ambientlayer.cpp
    core/map/ambientlayer.h
    core/map/map.cpp
    core/map/map.h
    core/map/mapreader.cpp
    core/map/mapreader.h
    core/map/position.cpp
    core/map/position.h
    core/map/properties.h
    core/map/tileset.h
    core/map/sprite/action.cpp
    core/map/sprite/action.h
    core/map/sprite/animatedsprite.cpp
    core/map/sprite/animatedsprite.h
    core/map/sprite/being.cpp
    core/map/sprite/being.h
    core/map/sprite/flooritem.cpp
    core/map/sprite/flooritem.h
    core/map/sprite/localplayer.cpp
    core/map/sprite/localplayer.h
    core/map/sprite/monster.cpp
    core/map/sprite/monster.h
    core/map/sprite/npc.cpp
    core/map/sprite/npc.h
    core/map/sprite/player.cpp
    core/map/sprite/player.h
    core/map/sprite/sprite.h
    core/map/sprite/spritedef.cpp
    core/map/sprite/spritedef.h
    core/sound/music.cpp
    core/sound/music.h
    core/sound/soundeffect.cpp
    core/sound/soundeffect.h
    core/utils/base64.cpp
    core/utils/base64.h
    core/utils/clipboard.cpp
    core/utils/clipboard.h
    core/utils/dtor.h
    core/utils/fastsqrt.h
    core/utils/gettext.h
    core/utils/lockedarray.h
    core/utils/metric.h
    core/utils/mutex.h
    core/utils/stringutils.cpp
    core/utils/stringutils.h
    core/utils/vector.cpp
    core/utils/vector.h
    core/utils/xml.cpp
    core/utils/xml.h
    eathena/beingmanager.cpp
    eathena/beingmanager.h
    eathena/flooritemmanager.cpp
    eathena/flooritemmanager.h
    eathena/game.cpp
    eathena/game.h
    eathena/party.cpp
    eathena/party.h
    eathena/playerrelations.cpp
    eathena/playerrelations.h
    eathena/statemanager.cpp
    eathena/statemanager.h
    eathena/db/colordb.cpp
    eathena/db/colordb.h
    eathena/db/effectdb.cpp
    eathena/db/effectdb.h
    eathena/db/emotedb.cpp
    eathena/db/emotedb.h
    eathena/db/itemdb.cpp
    eathena/db/itemdb.h
    eathena/db/iteminfo.h
    eathena/db/iteminfo.cpp
    eathena/db/monsterdb.cpp
    eathena/db/monsterdb.h
    eathena/db/monsterinfo.cpp
    eathena/db/monsterinfo.h
    eathena/db/npcdb.cpp
    eathena/db/npcdb.h
    eathena/db/skilldb.cpp
    eathena/db/skilldb.h
    eathena/gui/tabs/setup_game.cpp
    eathena/gui/tabs/setup_game.h
    eathena/gui/tabs/setup_players.cpp
    eathena/gui/tabs/setup_players.h
    eathena/gui/buy.cpp
    eathena/gui/buy.h
    eathena/gui/buysell.cpp
    eathena/gui/buysell.h
    eathena/gui/charcreate.cpp
    eathena/gui/charcreate.h
    eathena/gui/charselect.cpp
    eathena/gui/charselect.h
    eathena/gui/chat.cpp
    eathena/gui/chat.h
    eathena/gui/debugwindow.cpp
    eathena/gui/debugwindow.h
    eathena/gui/emotewindow.cpp
    eathena/gui/emotewindow.h
    eathena/gui/equipmentwindow.cpp
    eathena/gui/equipmentwindow.h
    eathena/gui/inventorywindow.cpp
    eathena/gui/inventorywindow.h
    eathena/gui/itempopup.cpp
    eathena/gui/itempopup.h
    eathena/gui/itemamount.cpp
    eathena/gui/itemamount.h
    eathena/gui/login.cpp
    eathena/gui/login.h
    eathena/gui/menubar.cpp
    eathena/gui/menubar.h
    eathena/gui/minimap.cpp
    eathena/gui/minimap.h
    eathena/gui/ministatus.cpp
    eathena/gui/ministatus.h
    eathena/gui/npcintegerdialog.cpp
    eathena/gui/npcintegerdialog.h
    eathena/gui/npclistdialog.cpp
    eathena/gui/npclistdialog.h
    eathena/gui/npcstringdialog.cpp
    eathena/gui/npcstringdialog.h
    eathena/gui/npctext.cpp
    eathena/gui/npctext.h
    eathena/gui/popupmenu.cpp
    eathena/gui/popupmenu.h
    eathena/gui/register.cpp
    eathena/gui/register.h
    eathena/gui/sell.cpp
    eathena/gui/sell.h
    eathena/gui/serverlistdialog.cpp
    eathena/gui/serverlistdialog.h
    eathena/gui/skill.cpp
    eathena/gui/skill.h
    eathena/gui/slotselection.cpp
    eathena/gui/slotselection.h
    eathena/gui/status.cpp
    eathena/gui/status.h
    eathena/gui/storagewindow.cpp
    eathena/gui/storagewindow.h
    eathena/gui/trade.cpp
    eathena/gui/trade.h
    eathena/gui/updatewindow.h
    eathena/gui/updatewindow.cpp
    eathena/gui/viewport.cpp
    eathena/gui/viewport.h
    eathena/handlers/emoteshortcut.h
    eathena/handlers/itemlinkhandler.cpp
    eathena/handlers/itemlinkhandler.h
    eathena/handlers/itemshortcut.h
    eathena/models/ignorechoiceslistmodel.h
    eathena/models/playerrelationlistmodel.h
    eathena/models/playertablemodel.cpp
    eathena/models/playertablemodel.h
    eathena/models/shoplistmodel.cpp
    eathena/models/shoplistmodel.h
    eathena/models/skilltablemodel.cpp
    eathena/models/skilltablemodel.h
    eathena/net/beinghandler.h
    eathena/net/beinghandler.cpp
    eathena/net/buysellhandler.h
    eathena/net/buysellhandler.cpp
    eathena/net/charserverhandler.h
    eathena/net/charserverhandler.cpp
    eathena/net/chathandler.h
    eathena/net/chathandler.cpp
    eathena/net/equipmenthandler.h
    eathena/net/equipmenthandler.cpp
    eathena/net/inventoryhandler.h
    eathena/net/inventoryhandler.cpp
    eathena/net/itemhandler.h
    eathena/net/itemhandler.cpp
    eathena/net/logindata.h
    eathena/net/loginhandler.h
    eathena/net/loginhandler.cpp
    eathena/net/maploginhandler.cpp
    eathena/net/maploginhandler.h
    eathena/net/messagehandler.cpp
    eathena/net/messagehandler.h
    eathena/net/messagein.cpp
    eathena/net/messagein.h
    eathena/net/messageout.cpp
    eathena/net/messageout.h
    eathena/net/network.cpp
    eathena/net/network.h
    eathena/net/npchandler.cpp
    eathena/net/npchandler.h
    eathena/net/partyhandler.cpp
    eathena/net/partyhandler.h
    eathena/net/playerhandler.cpp
    eathena/net/playerhandler.h
    eathena/net/protocol.h
    eathena/net/serverinfo.h
    eathena/net/skillhandler.cpp
    eathena/net/skillhandler.h
    eathena/net/tradehandler.cpp
    eathena/net/tradehandler.h
    eathena/structs/equipment.cpp
    eathena/structs/equipment.h
    eathena/structs/inventory.cpp
    eathena/structs/inventory.h
    eathena/structs/item.cpp
    eathena/structs/item.h
    eathena/structs/shopitem.cpp
    eathena/structs/shopitem.h
    eathena/widgets/emotecontainer.cpp
    eathena/widgets/emotecontainer.h
    eathena/widgets/emoteshortcutcontainer.cpp
    eathena/widgets/emoteshortcutcontainer.h
    eathena/widgets/itemcontainer.cpp
    eathena/widgets/itemcontainer.h
    eathena/widgets/itemshortcutcontainer.cpp
    eathena/widgets/itemshortcutcontainer.h
    eathena/widgets/shoplistbox.cpp
    eathena/widgets/shoplistbox.h
    engine.cpp
    engine.h
    main.cpp
    main.h
    options.h
    )

ADD_EXECUTABLE(aethyra ${SRCS})

TARGET_LINK_LIBRARIES(aethyra
    ${SDLGFX_LIBRARY}
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLNET_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${PNG_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${CURL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${GUICHAN_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${LIBINTL_LIBRARIES}
    )

INSTALL(TARGETS aethyra RUNTIME DESTINATION ${PKG_BINDIR})

IF (CMAKE_SYSTEM_NAME STREQUAL SunOS)
    # we expect the SMCgtxt package to be present on Solaris;
    # the Solaris gettext is not API-compatible to GNU gettext
    SET_TARGET_PROPERTIES(aethyra PROPERTIES LINK_FLAGS "-L/usr/local/lib")
ENDIF()

SET_TARGET_PROPERTIES(aethyra PROPERTIES COMPILE_FLAGS "${FLAGS}")
