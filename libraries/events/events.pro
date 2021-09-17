
include(../../mne-cpp.pri)

TEMPLATE = lib

CONFIG += skip_target_version_ext

QT += network concurrent
QT -= gui

DEFINES += EVENTS_LIBRARY

DESTDIR = $${MNE_LIBRARY_DIR}

TARGET = Events
TARGET = $$join(TARGET,,mnecpp,)
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

contains(MNECPP_CONFIG, static) {
    CONFIG += staticlib
    DEFINES += STATICBUILD
} else {
    CONFIG += shared
}

LIBS += -L$${MNE_LIBRARY_DIR}
CONFIG(debug, debug|release) {
    LIBS += -lmnecppUtilsd \
} else {
    LIBS += -lmnecppUtils \
}

SOURCES += \
    event.cpp \
    eventgroup.cpp \
    eventmanager.cpp \
    events_global.cpp \
    eventsharedmemmanager.cpp

HEADERS += \
    event.h \
    eventgroup.h \
    events_global.h \
    eventmanager.h \
    eventsharedmemmanager.h

INCLUDEPATH += $${MNE_INCLUDE_DIR}

contains(MNECPP_CONFIG, withCodeCov) {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
}

win32:!contains(MNECPP_CONFIG, static) {
    QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($${MNE_LIBRARY_DIR}/$${TARGET}.dll) $${MNE_BINARY_DIR}
}

macx {
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/
}

################################################## BUILD TIMESTAMP/HASH UPDATER ############################################

FILETOUPDATE = events_global.cpp

ALLFILES += $$HEADERS
ALLFILES += $$SOURCES
ALLFILES -= $$FILETOUPDATE
FileUpdater.target = phonyFileUpdater
FileUpdater.commands = touch $$PWD/$$FILETOUPDATE ; echo PASTA > phonyFileUpdater
FileUpdater.depends +=
for (IFILE, ALLFILES) {
    FileUpdater.depends += $$PWD/$$IFILE
}
PRE_TARGETDEPS += phonyFileUpdater
QMAKE_EXTRA_TARGETS += FileUpdater

