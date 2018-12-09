#ifndef WINDOWCOORDINATORPANE_H
#define WINDOWCOORDINATORPANE_H

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
#include <QResizeEvent>

#include <memory>
#include <functional>
#include <vector>
#include <sstream>

#include "orientation.h"
#include "filescoordinator.h"
#include "staticfunctions.h"
#include "windowcoordinator.h"
#include "tabselectorpane.h"
#include "removewindowdialog.h"
#include "infodialog.h"
#include "helpdialog.h"

class WindowCoordinator;
class WindowCoordSettings;
class RemoveWindowDialog;

class WindowCoordinatorPane : public QWidget
{
    Q_OBJECT
public:    
    explicit WindowCoordinatorPane(std::weak_ptr<WindowCoordinator> windowCoordinator,
                                   QList<QList<int>> splitterRatios,
                                   QWidget *parent = nullptr);
    ~WindowCoordinatorPane();

    QList<QList<int> > getSplitterRatios();
    void setSplitterRatios(QList<QList<int>> splitterRatios);

signals:
    void addWindow();
    void removeWindow(int id);
    void changeOrientation();
    void requestFullScreen();
    void splitterRatiosChangedSGNL(QList<QList<int>>);
public slots:
    void revalidateLayout();
    void clearSplitter();
//    void setFullScreen();
    void saveSplitterSizes();
    void splitterRatiosChanged();
protected:
    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;
private:
    QList<QList<int>> generateSplitterRatios();

    void resetMainLayout();
    void deleteMainLayout();

    void setToolBar();

    WindowCoordSettings generateSettings();

    QHBoxLayout* m_toolBar = nullptr;
    QVector<QWidget*> m_toolBarChilds;

    QSize TOOLBAR_ICON_SIZE = QSize(40,40);

    QVBoxLayout* m_vBox = nullptr;

    std::vector<QSplitter*> m_splitter;
    QList<QList<int>> m_splitterRatios;

    std::weak_ptr<WindowCoordinator> m_windowCoordinator;

    RemoveWindowDialog* m_removeDialog = nullptr;
    float m_dialogSizeFactor = 0.8f;
};

#endif // WINDOWCOORDINATORPANE_H
