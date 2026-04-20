QT += widgets sql network

CONFIG += c++17

TARGET = GameVersionByGoo
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow_v7v8.cpp \
    gamewidget_v7v8.cpp \
    database.cpp \
    networkmanager.cpp

HEADERS += \
    mainwindow.h \
    mainwindow_v7v8.h \
    gamewidget.h \
    gamewidget_v7v8.h \
    database.h \
    networkmanager.h
