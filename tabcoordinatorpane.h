#ifndef TABCOORDINATORPANE_H
#define TABCOORDINATORPANE_H

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

#include <memory>
#include <functional>
#include <vector>
#include <sstream>
#include <string>

#include "tabselectorpane.h"
#include "staticfunctions.h"
#include "tabcoordinator.h"

class TabCoordinator;
class TabSelectorPane;

class TabCoordinatorPane : public QWidget
{
    Q_OBJECT
public:
    explicit TabCoordinatorPane(std::weak_ptr<TabCoordinator> tabCoordinator,
                                int activeTabId,
                                QVector<QDir> tabLabels,
                                QWidget* parent = nullptr);
    ~TabCoordinatorPane();

    void setTabCoordinator(std::weak_ptr<TabCoordinator> tabCoordinator);

//    void setMainWidgetGetter(std::function<QWidget* ()> getMainWidget);

public slots:
    void revalidate();
    void updateTabLabels(QVector<QDir> tabLabels);
    void activeTabIdChanged(int id);
signals:
    void tabOkClicked(int id);
    void tabCloseClicked(int id);
    void tabAddClicked();

    void updateTabLabelsSGNL(QVector<QDir> tabLabels);
    void activeTabIdChangedSGNL(int id);

    void setFullScreen();

    void revalidateTabSelectorPane();
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    void revalidateMainWidget();

    void clearLayout();
    void deleteMainLayout();

    void setTabBar();

    QWidget* m_mainWidget;

    QVBoxLayout* m_mainLayout = nullptr;

    QVector<QDir> m_tabLabels;

    int m_curActiveTabId = 0;

//    std::function<QWidget* ()> m_getMainWidget = nullptr;

    TabSelectorPane* m_tabPane = nullptr;
    int m_tabPaneScrollOffsetX = 0;

    std::weak_ptr<TabCoordinator> m_tabCoordinator;
};

#endif // TABCOORDINATORPANE_H
