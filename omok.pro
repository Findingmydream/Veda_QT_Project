QT += core gui widgets sql network multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = Omok
TEMPLATE = app

SOURCES += \
    main.cpp \
    logindialog.cpp \
    mainwindow.cpp \
    gamewidget.cpp \
    database.cpp \
    networkmanager.cpp \
    profiledialog.cpp \
    playersearchdialog.cpp \
    cameracapturedialog.cpp \
    playerdetaildialog.cpp \
    recordstable.cpp

HEADERS += \
    logindialog.h \
    mainwindow.h \
    gamewidget.h \
    database.h \
    networkmanager.h \
    profiledialog.h \
    playersearchdialog.h \
    cameracapturedialog.h \
    playerdetaildialog.h \
    recordstable.h \
    titles.h

FORMS += \
    mainwindow.ui \
    logindialog.ui
