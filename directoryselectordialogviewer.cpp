#include "directoryselectordialogviewer.h"

DirectorySelectorDialogViewer::DirectorySelectorDialogViewer(
                        QString currentDirectory,
                        std::function<void (QString)> caller, QSizeF size, QWidget *parent)
    : m_curDir(currentDirectory),
      QGraphicsView(parent),
      m_size(size)
{
//    this->setAlignment(Qt::AlignTop);
//    this->setAlignment(Qt::AlignLeft);
    this->setAlignment(Qt::AlignLeft);

    this->setScene(&m_scene);

    loadBookmarks();

    QRectF rct(0,0,(qreal)viewport()->width(), (qreal)viewport()->height());

    QLinearGradient gradient(rct.topLeft(), rct.bottomRight());
    gradient.setColorAt(0, QColor(50,50,50, 255));
    gradient.setColorAt(1, QColor(0,0,0, 255));
    this->setBackgroundBrush(QBrush(gradient));

    this->horizontalScrollBar()->setVisible(false);

    m_caller = caller;

    rePaintCanvas();
}

DirectorySelectorDialogViewer::~DirectorySelectorDialogViewer()
{
    if(m_caller){
        m_caller = nullptr;
    }
}

void DirectorySelectorDialogViewer::rePaintCanvas()
{
    m_scene.clear();
    m_scene.setSceneRect(0,0, m_size.width()-verticalScrollBar()->width(), m_size.height());

    int* rowId = new int(0);
    // paint bookmarks:

    paintLabel(QString("bookmarks:"));
    ++(*rowId);
        foreach(QString bkmrkStr, m_bookmarks){
            paintBookmark(QFileInfo(bkmrkStr), (*rowId)++);
        }
    m_bookmarkAddBtn = new GraphicItemsBD::TextRect(QString("add current directory to bookmarks"),
                                        QSize(m_size.width()*0.8,m_rowHeight),
                                        QPoint(0,0),
                                        QColor(100,100,100, 255),QColor(0,0,0, 255),
                                        QColor(150,150,100, 255),QColor(50,50,0, 255),
                                        QColor(255,255,255,255));
    m_bookmarkAddBtn->setCallFunction([=](){showBookmarkDialog();});
    m_bookmarkAddBtn->setPosition(QPoint((m_size.width()-m_bookmarkAddBtn->boundingRect().width())*0.5,
                                   (*rowId+0.5)*m_rowHeight));
    m_scene.addItem(m_bookmarkAddBtn);

    (*rowId) += 2;


    // paint favourites:
    paintLabel(QString("favourites:"), (*rowId)++);
        paintStandardLocation(QStandardPaths::StandardLocation::DocumentsLocation   , rowId);
        paintStandardLocation(QStandardPaths::StandardLocation::MoviesLocation      , rowId);
        paintStandardLocation(QStandardPaths::StandardLocation::MusicLocation       , rowId);
        paintStandardLocation(QStandardPaths::StandardLocation::PicturesLocation    , rowId);
        paintStandardLocation(QStandardPaths::StandardLocation::DownloadLocation    , rowId);
        ++(*rowId);
        paintStandardLocation(QStandardPaths::StandardLocation::DesktopLocation     , rowId);
        paintStandardLocation(QStandardPaths::StandardLocation::HomeLocation        , rowId);
    ++(*rowId);

    // paint drives:
    paintLabel(QString("drives:"), (*rowId)++);
        QStorageInfo storage = QStorageInfo::root();
        paintFileInfo(QFileInfo(storage.rootPath()), (*rowId)++, 1, true);

        QList<QStorageInfo> mv = QStorageInfo::mountedVolumes();
        for(int i=0; i < mv.size(); i++){
            if(storage.rootPath() != mv[i].rootPath()){
                paintMountedDrive(QFileInfo(mv[i].rootPath()), (*rowId)++, 1);
//                paintFileInfo(QFileInfo(mv[i].rootPath()), (*rowId)++, 1, true);
            }
        }

    qreal paneHeight = (*rowId)*m_rowHeight;
    m_scene.setSceneRect(m_scene.sceneRect().x(),
                         m_scene.sceneRect().y(),
                         m_scene.sceneRect().width(),
                         paneHeight);

    delete rowId;
}

void DirectorySelectorDialogViewer::showBookmarkDialog()
{
    if( !m_curDir.isEmpty() && QFileInfo(m_curDir).exists() &&
            !m_bookmarks.contains(m_curDir)){
        m_bookmarks.append(m_curDir);
    }

    revalidateBookmarks();

    QTimer::singleShot(0,[=](){rePaintCanvas();});
}

void DirectorySelectorDialogViewer::revalidateBookmarks()
{
    int id=0;
    while(id < m_bookmarks.size()){
        if(m_bookmarks[id].isEmpty() || !(QFileInfo(m_bookmarks[id]).exists())){
            m_bookmarks.removeAt(id);
        }else{
            ++id;
        }
    }

    QFile file(m_bookmarks_res_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream( &file );

        foreach (auto& bkmrk, m_bookmarks) {
            if( !bkmrk.isEmpty() && QFileInfo(bkmrk).exists()){
                stream << bkmrk << tr("\n");
            }
        }
        stream.flush();
    }
    file.close();
}

void DirectorySelectorDialogViewer::loadBookmarks()
{
    m_bookmarks.clear();

    QDir dir;
    if( !dir.exists(tr("resources"))){
        dir.mkdir(tr("resources"));
    }
    if( !QFileInfo(m_bookmarks_res_path).exists() ){
        QFile file(m_bookmarks_res_path);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        file.close();
    }

    QFile file(m_bookmarks_res_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
//        qDebug() << line;
        if( !line.isEmpty() && QFileInfo(line).exists() &&
                !m_bookmarks.contains(line)){
            m_bookmarks.append(line);
        }
    }
    file.close();
}

void DirectorySelectorDialogViewer::
        paintLabel( QString str, int rowId, int colId){
                    QGraphicsTextItem* txtItem = new QGraphicsTextItem(str);
                    txtItem->setDefaultTextColor(QColor(255,255,255));
                    txtItem->setFont(StaticFunctions::getGoshFont(10));
                    txtItem->setPos(colId*m_colOffs, rowId*m_rowHeight);

    m_scene.addItem(txtItem);
}

void DirectorySelectorDialogViewer::paintFileInfo(QFileInfo fi, int rowId, int colId, bool drawAbsoluteFilePath)
{
    std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>> callFunction
                    = std::make_shared<DynamicFunctionCaller<QString, std::function<bool()>>>();

    callFunction->setFunction(QString("select"),
                              [=](){m_caller(fi.absoluteFilePath());return false;});

    GraphicsFile* gf = new GraphicsFile(fi, QSize(m_size.width()-40,m_rowHeight),
                                        rowId, m_rowHeight,
                                        colId, m_colOffs,
                                        callFunction);
    gf->setTextColor(QColor(255,255,255));
    gf->setDetailsTextColor(QColor(255,255,255,50));
    gf->setBackgroundColor(QColor(0,0,0, 50));
    gf->drawAbsoluteFilePath(drawAbsoluteFilePath);
    gf->setPosition(QPoint(0, rowId * m_rowHeight));
    gf->openOnDoubleClick(false);
    gf->rightClickEnabled(false);
    m_scene.addItem(gf);
}



void DirectorySelectorDialogViewer::paintBookmark(QFileInfo fi, int rowId, int colId, bool drawAbsoluteFilePath)
{
    paintFileInfo(QFileInfo(fi.absoluteFilePath()), rowId, colId, drawAbsoluteFilePath);

    GraphicItemsBD::TextRect* remRct = new GraphicItemsBD::TextRect(QString("X"),QSize(20,m_rowHeight),QPoint(m_size.width()-40, rowId*m_rowHeight),
                                    QColor(255,0,0),QColor(150,0,0),
                                    QColor(255,100,100),QColor(150,50,50),
                                    QColor(0,0,0));
    remRct->setCallFunction([=](){
        QTimer::singleShot(0,[=](){
            m_bookmarks.removeOne(fi.absoluteFilePath());
            revalidateBookmarks();
            rePaintCanvas();
        });
    });
    m_scene.addItem(remRct);
}

void DirectorySelectorDialogViewer::paintMountedDrive(QFileInfo fi, int rowId, int colId, bool drawAbsoluteFilePath)
{
    paintFileInfo(QFileInfo(fi.absoluteFilePath()), rowId, colId, drawAbsoluteFilePath);

    GraphicItemsBD::TextRect* remRct = new GraphicItemsBD::TextRect(QString("X"),QSize(20,m_rowHeight),QPoint(m_size.width()-40, rowId*m_rowHeight),
                                    QColor(255,0,0),QColor(150,0,0),
                                    QColor(255,100,100),QColor(150,50,50),
                                    QColor(0,0,0));
    remRct->setCallFunction([=](){
        QTimer::singleShot(0,[=](){
            StaticFunctions::unmountDrive(QDir(fi.absoluteFilePath()));
            rePaintCanvas();
        });
    });
    m_scene.addItem(remRct);
}

void DirectorySelectorDialogViewer::paintStandardLocation(QStandardPaths::StandardLocation loc,
                                                          int* rowId){
    QStringList pths = QStandardPaths::standardLocations(loc);
    foreach(QString pthStr, pths){
        paintFileInfo(QFileInfo(pthStr), (*rowId));
        ++(*rowId);
    }
}
