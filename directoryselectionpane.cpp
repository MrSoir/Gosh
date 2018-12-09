#include "directoryselectionpane.h"

DirectorySelectionPane::DirectorySelectionPane(QDir rootFolder, QWidget *parent)
    : m_scene(new QGraphicsScene()),
      QGraphicsView(parent),
      m_folder(rootFolder)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setScene(m_scene);
    setMaximumHeight(20);

    setFolder(m_folder);

    rePaintCanvas();
}

DirectorySelectionPane::~DirectorySelectionPane()
{
//    qDebug() << "in DirectorySelectionPane.DEstructor";

    m_cur_displd_folders.clear();

    m_undoEnabled = nullptr;
    m_onButtonClick = nullptr;
    m_onUndo = nullptr;

    m_scene->clear();
    this->setScene(nullptr);
}

void DirectorySelectionPane::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    rePaintCanvas();
}

void DirectorySelectionPane::updateGraphicsView()
{
    this->viewport()->update();
}

void DirectorySelectionPane::setFolder(QDir currentFolder)
{
    if(currentFolder.exists())
    {
        m_folder = currentFolder;
        m_cur_displd_folders.clear();
        do{
            m_cur_displd_folders.push_back( QDir(currentFolder) );
        }while(currentFolder.cdUp());
    }
    this->update();
}

void DirectorySelectionPane::undoEnabled(std::function<bool ()> caller)
{
    m_undoEnabled = caller;
}

void DirectorySelectionPane::onUndo(std::function<void ()> caller)
{
    m_onUndo = caller;
}
void DirectorySelectionPane::onButtonClick(std::function<void (QDir)> caller)
{
    m_onButtonClick = caller;
}

void DirectorySelectionPane::blockButtons(bool blockBtns)
{
    m_blockButtons = blockBtns;
}
bool DirectorySelectionPane::isBlocked()
{
    return m_blockButtons;
}

GraphicItemsBD::TextRect* genTextRect(QString str, QPoint pos = QPoint(0,0))
{
    QFontMetrics fm(StaticFunctions::getGoshFont());
    QSize size(fm.width(str), fm.height());
    GraphicItemsBD::TextRect* tr =  new GraphicItemsBD::TextRect(   str,
                                    size,
                                    pos,
                                    QColor(255,255,255, 255),
                                    QColor(255,255,255, 255),
                                    QColor(230,230,255, 255),
                                    QColor(230,230,255, 255)
                                  );
    return tr;
}

void DirectorySelectionPane::revalidate(){
    rePaintCanvas();
}
GraphicItemsBD::PixmapRect* DirectorySelectionPane::createUndoPixmapItem(const QColor& undoColor, QPoint pos, int undo_pixmap_width)
{
    QString undo_pixmap_path = QString("%1%2%3").arg("pics").arg(QDir::separator()).arg("undo_icon.png");
    GraphicItemsBD::PixmapRect* pr = new GraphicItemsBD::PixmapRect(QPixmap(undo_pixmap_path), QSize(undo_pixmap_width, undo_pixmap_width), pos,
                                    undoColor,undoColor);
    pr->setCallFunction([=](){
        if( !isBlocked() )
        {
            if(m_onUndo)
                m_onUndo();
            emit undo();
        }
    });

//    pr->onPressed(m_onUndo);
    return pr;
}
void DirectorySelectionPane::rePaintCanvas()
{
    m_scene->clear();
    m_scene->setSceneRect(QRect(0,0, this->viewport()->width(), this->viewport()->height()));

    QVector<GraphicItemsBD::TextRect*> dirGItms;

    dirGItms.append( genTextRect("...", QPoint(4, 0)) );
    dirGItms[0]->setCallFunction(
            [=](){
                QString absFilePath = showFileSelectorDialog(m_folder.absolutePath());
                QDir dir(absFilePath);
                if( !absFilePath.isEmpty() && dir.exists())
                {
                    if(m_onButtonClick)
                        m_onButtonClick(dir);
                    emit buttonClicked(dir);
                }
            }
    );
    m_scene->addItem( dirGItms[0] );
    for(int i=0; i < m_cur_displd_folders.size(); ++i)
    {
        addDirGI(m_cur_displd_folders[i], dirGItms);
        m_scene->addItem( dirGItms[i+1] );
    }

    float padding = 4.0f;
    float dir_items_widh = padding;
    for(unsigned int i=0; i < dirGItms.size(); ++i)
    {
        dir_items_widh += dirGItms[i]->boundingRect().width() + padding;// i < dirGItms.size()-1 ? padding : 0.0f;
    }

    int undo_pixmap_width = 15;

    QColor undoColor = m_undoEnabled && m_undoEnabled() ? QColor(230,230,255, 255) :
                                                          QColor(100,100,100, 255);

    float paddingTop = 1.0f;
    if(dir_items_widh + undo_pixmap_width > this->viewport()->width())
    {
        float layX = this->viewport()->width() - padding;

        for(int i=0; i < dirGItms.size(); ++i)
        {
            layX -= dirGItms[i]->boundingRect().width();
            dirGItms[i]->setPosition( QPoint(layX, paddingTop) );
            layX -= padding;
        }
        GraphicItemsBD::PixmapRect* pr = createUndoPixmapItem(undoColor, QPoint(padding, 2));
//        PixmapRect* pr = new PixmapRect(QPixmap(undo_pixmap_path), QSize(undo_pixmap_width, undo_pixmap_width),,
//                                        undoColor,undoColor);
        QGraphicsRectItem* whiteRect = new QGraphicsRectItem(0, paddingTop,
                                                             pr->boundingRect().x()+pr->boundingRect().width() + 2,
                                                             pr->boundingRect().height());
        whiteRect->setBrush(QBrush(QColor(255,255,255, 255)));
        whiteRect->setPen(QPen(QColor(255,255,255, 255)));

        m_scene->addItem(whiteRect);
        m_scene->addItem(pr);
    }else
    {
        float layX = padding;
        GraphicItemsBD::PixmapRect* pr = createUndoPixmapItem(undoColor, QPoint(layX, 2));

        m_scene->addItem(pr);

        layX += undo_pixmap_width + padding;

        for(int i=dirGItms.size()-1; i >= 0; --i)
        {
            dirGItms[i]->setPosition( QPoint(layX, paddingTop) );
            layX += dirGItms[i]->boundingRect().width() + padding;
        }
    }

//    dirGItms.clear();
}


void DirectorySelectionPane::addDirGI(const QDir &dir, QVector<GraphicItemsBD::TextRect*>& dirGItms)
{
    QString rootName = (dir.isRoot() && dir.dirName().isEmpty()) ? QString("root") : dir.dirName();

    GraphicItemsBD::TextRect* tr = genTextRect( rootName );
    tr->setText( rootName, 4 );
    tr->setFont(StaticFunctions::getGoshFont(8, QFont::Normal));
    tr->setCallFunction(
                    [=](){
                        if( !isBlocked() )
                        {
                            if(m_onButtonClick)
                                m_onButtonClick( QDir(dir) );
                            emit buttonClicked(dir);
                        }
                    }
                );
    dirGItms.append( tr );
}

QString DirectorySelectionPane::showFileSelectorDialog(QString absPath)
{
    return StaticFunctions::getTextViaDialog(absPath, QObject::tr("select path"));
}
