#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include <QDir>
#include <QVector>
#include <QFileInfo>
#include <QDateTime>
#include <QTime>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QMainWindow>
#include <QStatusBar>
#include <QIcon>
#include <memory>

#include "staticfunctions.h"
#include "tabcoordinator.h"
//#include "windowcoordinator.h"
#include "custommainwindowbd.h"
#include "orderby.h"

#include "filehandler.h"

std::shared_ptr<TabCoordinator> tabCoordinator;
//std::shared_ptr<WindowCoordinator> windowCoordinator;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("Gosh");

    qRegisterMetaType<ORDER_BY>("ORDER_BY");
    qRegisterMetaType<QFileInfo>("QFileInfo");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QDir>("QDir");

    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    qRegisterMetaType<QVector<QDir>>("QVector<QDir>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QList<QList<int>>>("QList<QList<int>>");

    FileHandler::initialize();


//    app.setWindowIcon(QIcon(QString("%1%2%3")
//                                .arg(QString("pics"))
//                                .arg(QDir::separator())
//                                .arg(QString("MrSoir_antique_big.png"))));

    QFile styleFile(QString("styles%1%2").arg(QDir::separator()).arg("stylesheet.qss"));
    qDebug() << "styleFile.exists: " << styleFile.exists();
    if(styleFile.open(QIODevice::ReadOnly))
    {
        QTextStream textStream(&styleFile);
        QString styleSheet = textStream.readAll();
        styleFile.close();
        qDebug() << "styleSheet.length: " << styleSheet.length();
//        qDebug() << styleSheet;
        app.setStyleSheet(styleSheet);
    }

    QMainWindow* window = new QMainWindow();

//    windowCoordinator = std::make_shared<WindowCoordinator>(nullptr);
//    windowCoordinator->setSelf( std::weak_ptr<WindowCoordinator>(windowCoordinator) );
    tabCoordinator = std::make_shared<TabCoordinator>(nullptr);
    tabCoordinator->setSelf( std::weak_ptr<TabCoordinator>(tabCoordinator) );

    std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> caller
        = std::make_shared<DynamicFunctionCaller<QString, std::function<void()>>>();
    caller->setFunction(QString("setFullScreen"), [=](){window->showFullScreen();});
    caller->setFunction(QString("setMaximized"), [=](){window->showMaximized();});
//    windowCoordinator->setFullScreenCaller(caller);
    tabCoordinator->setFullScreenCaller(caller);
//    window->setCentralWidget( windowCoordinator->getWindowCoordinatorPane() );
    window->setCentralWidget( tabCoordinator->getTabCoordinatorPane() );
    qDebug() << "in main.cpp -> added tabCoordinator";

    StaticFunctions::setIconToWidget(window);

//    window->showFullScreen();
    window->showMaximized();
    window->show();

    return app.exec();
}
