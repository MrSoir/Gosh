#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>
#include <QSize>
#include <QRect>
#include <QPoint>
#include <QFileInfo>
#include <QDebug>
#include <QObject>
#include <functional>
#include <QTimer>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsItemGroup>
#include <QWidget>
#include <QDir>
#include <QPixmap>
#include <memory>
#include <thread>
#include <atomic>

#include "math.h"
#include "filescoordinator.h"
#include "fileinfobd.h"
#include "graphicsfile.h"
#include "graphicsviewupdater.h"
#include "limits.h"
#include "dynamicfunctioncaller.h"
#include "directoryselectordialog.h"
#include "staticfunctions.h"
#include "searchfiledialog.h"
#include "customgraphicitems.h"
#include "menubar.h"
#include "waitingbargraphicsitem.h"

class GraphicsItemBD;
//class TextRect;
class MenuBar;
class SearchMenuBD;
class WindowSelector;
class FilesCoordinator;

enum FILE_ACTION{
    COPY, CUT, PASTE, DUPLICATE,
    ZIP, UNZIP,
    RENAME,
    DELETE,
    OPEN, OPEN_WITH,
    TERMINAL,
    DETAILS,
    ELAPSE, ELAPSE_REC, // ELAPSE_REC == elapse RECursively
    TAB, CLOSE_TAB,
    LOAD,
    COLLAPSE, COLLAPSE_REC, // COLLAPSE_REC == collapse RECursively
    PATH,
    CANCEL_CURRENT_ACTION,
    REQUEST_FOCUS
};

class GraphicsView : public QGraphicsView,
                     public GraphicsViewUpdater
{
    Q_OBJECT
public:
    explicit GraphicsView(int hBarValue = 0,
                          int vBarValue = 0,
                          int zoomFactor = 9,
                          QWidget* parent = nullptr);
    ~GraphicsView();

//    void setRoot(std::weak_ptr<FileInfoBD> fi);

    void updateGraphicsView() override;

    void setFilesCoordinator(std::weak_ptr<FilesCoordinator> filesCoor);

    int getVScrollBarValue();
    int getHScrollBarValue();

signals:
    void requestCloseCurrentTab();
    void requestOpenFoldersInTab();

    void nextSearchResultSGNL(); // -> nextSearchResult()
    void prevSearchResultSGNL(); // -> previousSearchResult()
    void closeSearchMenuSGNL();
    void searchForKeyWord(QString keyword, bool deepsearch);

    void elapseAllFoldersOfDepthId(int id);

//    -------------------------

    void selectEntireContent();
    void clearSelection();
    void selectButtonUp(bool contrlPrsd, bool shiftPrsd);
    void selectButtonDown(bool contrlPrsd, bool shiftPrsd);

    void selectContent(QString entry, bool isFolder, bool contrlPrsd, bool shiftPrsd);
//    void selectFromDisplayedContent(QFileInfo fi, bool contrlPrsd, bool shiftPrsd);
    void sortFromDisplayedContent(QFileInfo fi, ORDER_BY order);

    void copySelectedContent();
    void cutSelectedContent();
    void duplicateSelectedContent();
    void openSelectedContent();
    void openSelectedContentWith();

    void renameSelectedContent();
    void pasteFromClipboard();
    void paste(QString dropStr, QString targetPath);
    void deleteSelectedContent();

    void showDetailsOfSelectedContent();

    void zipSelectedContent();
    void unzipSelectedContent();

    void createNewFolderSGNL(); // -> createNewFolder()
    void createNewFileSGNL(); // -> createNewFile()

    void elapseSelectedFoldersRecursively();
    void elapseSelectedFolders();
    void collapseSelectedFoldersRecursively();
    void collapseSelectedFolders();
    void copySelectedFilePathToClipboard();

    void killCurrentBlockingAction(); // -> killCurrentBlockingActionSLT()

    void requestFocusSGNL(); // -> requestFocus()

    void initDragging(QString draggingSource);

    void keyPressed(int keyId);

    void setSelectionToRoot();
    void setRootFolder(QDir newRoot);


//    void sort(FileInfoBD fiBD, ORDER_BY order);
    void sortAllFolders(ORDER_BY order);

    void openTerminalSGNL(); // -> openTerminal
    void openSelection();
    void setParentToRoot();

    void zoomFactorChanged(int newZoomFactor);

    void showHiddenFilesSGNL(bool showHiddenFiles);
public slots:
    void requestFocus();

//    void folderChanged(std::weak_ptr<const FileInfoBD> f = std::weak_ptr<FileInfoBD>());
    void revalidate();

    void focusId(int id, bool repaintAnyway = false);

    void vScrollValueChanged(int newValue);
    void hScrollValueChanged(int newValue);
    void setHBarValue(int hBarValue);
    void setVBarValue(int vBarValue);

//    void setWaitingAnimation();
    void startWaitingAnimation();
    void killWaitingAnimation();

    void handleSearchKeyword(QString keyword, bool deepSearch);
protected:
     void EnterPressedBD(QKeyEvent* event);
     void keyPressEvent(QKeyEvent *event);
     void keyReleaseEvent(QKeyEvent *event);

     void resizeEvent(QResizeEvent *event);

     void mousePressEvent(QMouseEvent *event);
//     void mouseReleaseEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);

     void enterEvent(QEvent *event);
     void leaveEvent(QEvent* event);
private:
    void paintTopRectangle(const QPointF& center,
                           const QSize& size);

    int getFirstToDispFi();
    int getLastToDispFi();
    bool viewPortOutOfDisplayedRange();

    qreal getDisplayableHeight();
    void rePaintCanvas();

    void setWaitingBarSizeAndPos();

    void paintFileInfo(const QFileInfo& fi, int rowId=0, int colId=0,
                       std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>> caller =
                            std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>>(),
                       std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>> sortCaller
                            = std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>>());

    qreal getRelativeHorBarValue();
    qreal getRelativeVerBarValue();
    int getAbsoluteHorBarValue();
    int getAbsoluteVerticalBarValue();
    int getViewportYOffset();

    void addSearchMenu();
    void launchSearchMode();
    void nextSearchResult();
    void prevSearchResult();
    void closeSearchMenu();

    QString getCurrentSearchResult();
    long getIndexOfCurrentSearchResult();
    long getSearchIndex();
    long getSearchResultsCount();

    void addMenuBar();
    void closeMenuBar();

    void executeFileAction(FILE_ACTION action);
    void executeFileAction(std::function<void (std::shared_ptr<FilesCoordinator>)> fctn);

    void addContentBar();
    void closeContentBar();
    void addElapseBar();

    void zoomOut();
    void zoomIn();

    void sortAllFoldersDialog();

    void createNewFolder();
    void createNewFile();

    void openTerminal();

    void showHiddenFiles();

    void showRootSelector();

    void revalidateRowHeight();

    bool inSearchMode();

    QGraphicsScene m_scene;

    int m_fontSize;

    int m_rowHeight = 20;
    int m_colOffs = 15;

    int m_firstDispFI = 0;
    int m_lastDispFI = 0;
    int m_curDispFI = 0;
    int m_fileCount = 0;
    int m_fileMaxCount = 500;
    int m_filePuffer = 200;

    QGraphicsItemGroup* m_graphicsGroup = new QGraphicsItemGroup();

    SearchMenuBD* m_searchMenu = nullptr;
    MenuBar* m_menuBar = nullptr;
    MenuBar* m_contBar = nullptr;

    QPainterPath* m_upperRect = nullptr;
    int m_upperRectWidth = 40;
    QPoint* m_mouseP = nullptr;

    void closeAllSubMenus();

    bool m_paintUpperRect = false;
//    bool m_paintSearchMenu = false;
    bool m_paintMenuBar = false;
    bool m_paintContBar = false;
    int m_searchMenuHeight = 50;
    int m_menuBarHeight = 40;
    int m_contBarWidth = 45;

    bool m_shwRtSel = true;
    int m_rootSelWidth = 80;
    int m_rootSelHeight = 20;
    int m_elapseBarHeight = 15;
    QColor m_elapseCol1 = QColor(255,255,255, 255),
           m_elapseCol2 = QColor(200,255,200, 255);

    std::weak_ptr<FilesCoordinator> m_filesCoord = std::weak_ptr<FilesCoordinator>();
    std::shared_ptr<FilesCoordinator> lockFilesCoordinator();

    std::atomic<bool> m_isLoading;
    std::atomic<int> m_loadingId;
    int m_loadingLength = 90;
    int m_loadingPeriodMS = 20;
    QTimer* m_animationTimer;
    WaitingBarGraphicsItem* m_waitingBar;

    QSize m_cancelBtnSize;
    GraphicItemsBD::PixmapRect* m_cancelBtn = nullptr;
    bool m_waitingBarIsAddedToScene = false;
    int m_waitingBarHeight = 40;

    RectColor m_rectColor;
};

#endif // GRAPHICSVIEW_H
