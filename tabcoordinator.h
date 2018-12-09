#ifndef TABCOORDINATOR_H
#define TABCOORDINATOR_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPaintEvent>
#include <QPainter>
#include <QDialog>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QLayoutItem>
#include <QLayout>

#include <memory>
#include <functional>
#include <vector>

#include "tabcoordinatorpane.h"

#include "staticfunctions.h"
#include "dynamicfunctioncaller.h"
#include "windowcoordinator.h"

class TabCoordinatorPane;

class TabCoordinator : public QObject
{
    Q_OBJECT
public:
    explicit TabCoordinator(QObject *parent = nullptr);
    ~TabCoordinator();

    void setFullScreenCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> fullScreenCaller);

    QWidget* getCenterWidgetToDisplay();
    QWidget* resetCenterWidgetToDisplay();

    TabCoordinatorPane* getTabCoordinatorPane();

    void setSelf(std::weak_ptr<TabCoordinator> self);

    void setFullScreen();

signals:
    void revalidateTabCoordinatorPane();
    void labelsChanged(QVector<QDir> labels);
    void activeTabIdChanged(int id);
public slots:
    void addTab();
    void setTab(int id);
    void removeTab(int id);

    void closeActiveTab();
    void openFoldersInNewTab(QVector<QDir> foldersToOpen);

    void revalidateLabels();
private:
    QVector<QDir> generateLabels();

    void addTabHelper(QVector<QDir>* initPaths = nullptr);


    std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> m_fullScreenCaller
             = std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>>();
    bool m_isFullScreen = false;

    QVector<std::shared_ptr<WindowCoordinator>> m_windows;

    std::weak_ptr<WindowCoordinator> m_currentlyDisplWindow = std::weak_ptr<WindowCoordinator>();
    int m_curWindowId = -1;

    std::weak_ptr<TabCoordinator> m_self = std::weak_ptr<TabCoordinator>();
};

#endif // TABCOORDINATOR_H
