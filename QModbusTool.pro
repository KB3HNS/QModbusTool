QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17 -Wextra -Wpedantic

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += QCUSTOMPLOT_USE_OPENGL  # enable OpenGL accelleration for trend

SOURCES += \
    coils_display.cpp \
    exceptions.cpp \
    holding_register_display.cpp \
    main.cpp \
    mainwindow.cpp \
    modbusthread.cpp \
    register_display.cpp \
    scheduler.cpp \
    metadata_wrapper.cpp \
    metadata_structs.cpp \
    inputs_display.cpp \
    csv_importer.cpp \
    trend_window.cpp \
    base_dialog.cpp \
    trend_line.cpp \
    configure_trend_line.cpp \
    configure_trend.cpp

HEADERS += \
    coils_display.h \
    exceptions.h \
    holding_register_display.h \
    mainwindow.h \
    modbusthread.h \
    register_display.h \
    scheduler.h \
    write_event.h \
    metadata_wrapper.h \
    metadata_structs.h \
    inputs_display.h \
    csv_importer.h \
    trend_window.h \
    base_dialog.h \
    trend_line.h \
    configure_trend_line.h \
    configure_trend.h

FORMS += \
    mainwindow.ui \
    configure_trend_line.ui \
    configure_trend.ui

TRANSLATIONS += \
    QModbusTool_en_US.ts

LIBS += \
    -L/usr/local/lib -lmodbus -ldl -lqtcsv -lqcustomplot

INCLUDEPATH += \
    /usr/local/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = QModbusTool.ico

RESOURCES += \
    resources.qrc
