# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-mupen64plus-gui-sailfishos

CONFIG += sailfishapp

SOURCES += src/harbour-mupen64plus-gui-sailfishos.cpp \
    src/gamelauncher.cpp \
    src/core_interface.cpp \
    src/osal_dynamiclib_unix.c \
    src/osal_files_unix.c \
    src/recents_handler.cpp \
    src/settingsmanager.cpp \
    src/emulator_thread.cpp

OTHER_FILES += qml/harbour-mupen64plus-gui-sailfishos.qml \
    rpm/harbour-mupen64plus-gui-sailfishos.changes.in \
    rpm/harbour-mupen64plus-gui-sailfishos.spec \
    rpm/harbour-mupen64plus-gui-sailfishos.yaml \
    translations/*.ts \
    harbour-mupen64plus-gui-sailfishos.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-mupen64plus-gui-sailfishos-de.ts

DISTFILES += \
    qml/pages/GameSelector.qml \
    qml/cover/Cover.qml \
    qml/pages/FilePicker.qml \
    qml/pages/ConfigurePlugins.qml \
    qml/pages/Error.qml \
    qml/pages/ConfigureRsp.qml \
    qml/pages/ConfigureInput.qml \
    qml/pages/ConfigureAudio.qml \
    qml/pages/ConfigureCore.qml \
    qml/pages/ConfigurePlugin.qml \
    qml/pages/ConfigureVideo.qml

HEADERS += \
    src/gamelauncher.h \
    src/core_interface.h \
    src/osal_dynamiclib.h \
    src/osal_files.h \
    src/osal_preproc.h \
    src/version.h \
    src/debug.h \
    src/recents_handler.h \
    src/settingsmanager.h \
    src/emulator_thread.h

INCLUDEPATH += ../mupen64plus-core/src/api /usr/include/mupen64plus /usr/include/audioresource-qt

LIBS += -ldl -laudioresource-qt

QT += multimedia
