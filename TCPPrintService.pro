QT += core network
QT -= gui

TARGET = TCPPrintService

CONFIG += c++11
CONFIG += console qt
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += LOG_TO_CONSOLE

INCLUDEPATH += \
    src \

include(../qt-solutions/qtservice/src/qtservice.pri)

LIBS +=-lwinspool

SOURCES += src/main.cpp \
    src/printerdaemon.cpp \
    src/printerservice.cpp \
    src/logger.cpp

HEADERS += \
    src/printerdaemon.h \
    src/printerservice.h \
    src/logger.h
