QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aimanager.cpp \
    audiomanager.cpp \
    combatsystem.cpp \
    effect.cpp \
    enemy.cpp \
    entity.cpp \
    entitymanager.cpp \
    gameapplication.cpp \
    gamemap.cpp \
    gamewindow.cpp \
    interactiveobject.cpp \
    inventorysystem.cpp \
    item.cpp \
    main.cpp \
    pet.cpp \
    player.cpp \
    potion.cpp \
    projectile.cpp \
    shop.cpp \
    throwable.cpp \
    upgradesystem.cpp \
    wall.cpp \
    weapon.cpp

HEADERS += \
    aimanager.h \
    audiomanager.h \
    combatsystem.h \
    effect.h \
    enemy.h \
    entity.h \
    entitymanager.h \
    gameapplication.h \
    gamemap.h \
    gamewindow.h \
    interactiveobject.h \
    inventorysystem.h \
    item.h \
    pet.h \
    player.h \
    potion.h \
    projectile.h \
    shop.h \
    throwable.h \
    upgradesystem.h \
    wall.h \
    weapon.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=

