/****************************************************************************
** Meta object code from reading C++ file 'filescoordinator.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../filescoordinator.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filescoordinator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FilesCoordinator_t {
    QByteArrayData data[14];
    char stringdata0[188];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FilesCoordinator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FilesCoordinator_t qt_meta_stringdata_FilesCoordinator = {
    {
QT_MOC_LITERAL(0, 0, 16), // "FilesCoordinator"
QT_MOC_LITERAL(1, 17, 19), // "repaintFolderViewer"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 19), // "setWaitingAnimation"
QT_MOC_LITERAL(4, 58, 4), // "wait"
QT_MOC_LITERAL(5, 63, 13), // "folderChanged"
QT_MOC_LITERAL(6, 77, 25), // "std::weak_ptr<FileInfoBD>"
QT_MOC_LITERAL(7, 103, 1), // "f"
QT_MOC_LITERAL(8, 105, 13), // "folderElapsed"
QT_MOC_LITERAL(9, 119, 14), // "sortingChanged"
QT_MOC_LITERAL(10, 134, 21), // "addDirectoryToWatcher"
QT_MOC_LITERAL(11, 156, 9), // "directory"
QT_MOC_LITERAL(12, 166, 16), // "directoryChanged"
QT_MOC_LITERAL(13, 183, 4) // "path"

    },
    "FilesCoordinator\0repaintFolderViewer\0"
    "\0setWaitingAnimation\0wait\0folderChanged\0"
    "std::weak_ptr<FileInfoBD>\0f\0folderElapsed\0"
    "sortingChanged\0addDirectoryToWatcher\0"
    "directory\0directoryChanged\0path"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FilesCoordinator[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x0a /* Public */,
       3,    1,   50,    2, 0x0a /* Public */,
       5,    1,   53,    2, 0x0a /* Public */,
       8,    1,   56,    2, 0x0a /* Public */,
       9,    1,   59,    2, 0x0a /* Public */,
      10,    1,   62,    2, 0x0a /* Public */,
      12,    1,   65,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   13,

       0        // eod
};

void FilesCoordinator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FilesCoordinator *_t = static_cast<FilesCoordinator *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->repaintFolderViewer(); break;
        case 1: _t->setWaitingAnimation((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->folderChanged((*reinterpret_cast< std::weak_ptr<FileInfoBD>(*)>(_a[1]))); break;
        case 3: _t->folderElapsed((*reinterpret_cast< std::weak_ptr<FileInfoBD>(*)>(_a[1]))); break;
        case 4: _t->sortingChanged((*reinterpret_cast< std::weak_ptr<FileInfoBD>(*)>(_a[1]))); break;
        case 5: _t->addDirectoryToWatcher((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->directoryChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject FilesCoordinator::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FilesCoordinator.data,
      qt_meta_data_FilesCoordinator,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *FilesCoordinator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FilesCoordinator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FilesCoordinator.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "FolderListener"))
        return static_cast< FolderListener*>(this);
    return QObject::qt_metacast(_clname);
}

int FilesCoordinator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
