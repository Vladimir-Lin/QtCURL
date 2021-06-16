NAME         = QtCURL
TARGET       = $${NAME}
QT           = core
QT          -= gui
QT          -= script

load(qt_build_config)
load(qt_module)

INCLUDEPATH += $${PWD}/../../include/QtCURL
INCLUDEPATH += $${PWD}/../../include/QtCURL/curl
INCLUDEPATH += $${PWD}/../../include/QtCURL/ssh2

HEADERS     += $${PWD}/../../include/QtCURL/qtcurl.h
HEADERS     += $${PWD}/../../include/QtCURL/curl/*
HEADERS     += $${PWD}/../../include/QtCURL/ssh2/*

SOURCES     += qtcurl.cpp

OTHER_FILES += $${PWD}/../../include/$${NAME}/headers.pri

win32 {

CURLLIB      = libcurl
SSH2LIB      = libssh2

CONFIG(debug, debug|release) {

  LIBS      += -l$${SSH2LIB}d
  LIBS      += -l$${CURLLIB}d

} else {

  LIBS      += -l$${SSH2LIB}
  LIBS      += -l$${CURLLIB}

}

}

include ($${PWD}/../../doc/Qt/Qt.pri)
