/****************************************************************************
** Meta object code from reading C++ file 'combatsystem.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../combatsystem.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'combatsystem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12CombatSystemE_t {};
} // unnamed namespace

template <> constexpr inline auto CombatSystem::qt_create_metaobjectdata<qt_meta_tag_ZN12CombatSystemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CombatSystem",
        "playerDamageDealt",
        "",
        "Player*",
        "source",
        "Enemy*",
        "target",
        "damage",
        "enemyDamageDealt",
        "playerRangeDamageDealt",
        "enemyRangeDamageDealt",
        "skillCast",
        "skillId",
        "QPointF",
        "position",
        "areaDamageApplied",
        "center",
        "radius",
        "projectileCreated",
        "Projectile*",
        "projectile"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'playerDamageDealt'
        QtMocHelpers::SignalData<void(Player *, Enemy *, int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'enemyDamageDealt'
        QtMocHelpers::SignalData<void(Enemy *, Player *, int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 4 }, { 0x80000000 | 3, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'playerRangeDamageDealt'
        QtMocHelpers::SignalData<void(Player *, Enemy *, int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'enemyRangeDamageDealt'
        QtMocHelpers::SignalData<void(Enemy *, Player *, int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 4 }, { 0x80000000 | 3, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'skillCast'
        QtMocHelpers::SignalData<void(int, const QPointF &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 }, { 0x80000000 | 13, 14 },
        }}),
        // Signal 'areaDamageApplied'
        QtMocHelpers::SignalData<void(const QPointF &, int, int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 16 }, { QMetaType::Int, 17 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'projectileCreated'
        QtMocHelpers::SignalData<void(Projectile *)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CombatSystem, qt_meta_tag_ZN12CombatSystemE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CombatSystem::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CombatSystemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CombatSystemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12CombatSystemE_t>.metaTypes,
    nullptr
} };

void CombatSystem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CombatSystem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->playerDamageDealt((*reinterpret_cast<std::add_pointer_t<Player*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Enemy*>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 1: _t->enemyDamageDealt((*reinterpret_cast<std::add_pointer_t<Enemy*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Player*>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 2: _t->playerRangeDamageDealt((*reinterpret_cast<std::add_pointer_t<Player*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Enemy*>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 3: _t->enemyRangeDamageDealt((*reinterpret_cast<std::add_pointer_t<Enemy*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Player*>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 4: _t->skillCast((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[2]))); break;
        case 5: _t->areaDamageApplied((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 6: _t->projectileCreated((*reinterpret_cast<std::add_pointer_t<Projectile*>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(Player * , Enemy * , int )>(_a, &CombatSystem::playerDamageDealt, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(Enemy * , Player * , int )>(_a, &CombatSystem::enemyDamageDealt, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(Player * , Enemy * , int )>(_a, &CombatSystem::playerRangeDamageDealt, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(Enemy * , Player * , int )>(_a, &CombatSystem::enemyRangeDamageDealt, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(int , const QPointF & )>(_a, &CombatSystem::skillCast, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(const QPointF & , int , int )>(_a, &CombatSystem::areaDamageApplied, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (CombatSystem::*)(Projectile * )>(_a, &CombatSystem::projectileCreated, 6))
            return;
    }
}

const QMetaObject *CombatSystem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CombatSystem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CombatSystemE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CombatSystem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CombatSystem::playerDamageDealt(Player * _t1, Enemy * _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void CombatSystem::enemyDamageDealt(Enemy * _t1, Player * _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void CombatSystem::playerRangeDamageDealt(Player * _t1, Enemy * _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}

// SIGNAL 3
void CombatSystem::enemyRangeDamageDealt(Enemy * _t1, Player * _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}

// SIGNAL 4
void CombatSystem::skillCast(int _t1, const QPointF & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void CombatSystem::areaDamageApplied(const QPointF & _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3);
}

// SIGNAL 6
void CombatSystem::projectileCreated(Projectile * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
