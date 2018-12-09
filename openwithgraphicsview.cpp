#include "openwithgraphicsview.h"



OpenWithGraphicsView::OpenWithGraphicsView(const QFileInfo& fileToOpen,
                                           std::vector<ProgramMimeAssociation> &programHandlers,
                                           QWidget *parent)
    : QGraphicsView(parent),
      m_fileToOpen(fileToOpen),
      m_programHandlers(programHandlers)
{
    this->setScene(&m_scene);

    this->setAlignment(Qt::AlignTop);

    for(int i=0; i < m_programHandlers.size(); ++i)
    {
        QString iconPath = m_programHandlers.at(i).getIconPath();
        if( !QFileInfo(iconPath).exists() )
            iconPath = m_defaultIconPath;

        m_iconPaths.push_back(iconPath);
        m_programNames.push_back( QString::fromStdString(m_programHandlers[i].name) );

        qDebug() << "[" << i << "]: " << iconPath << "  |   " << m_programNames[m_programNames.size()-1];
    }

    this->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

    repaint();
}

OpenWithGraphicsView::~OpenWithGraphicsView()
{
    m_scene.clear();
    items.clear();
}

void OpenWithGraphicsView::repaint()
{
    m_scene.clear();

    int paddingX = 10;
    int paddingY = 5;

    int yOffs = 10;
    qDebug() << "in repaint: width: " << this->viewport()->width();

    GraphicItemsBD::IconAndLabelItem* itm =
            new GraphicItemsBD::IconAndLabelItem(m_fileToOpen.absoluteFilePath(), true,
                                                 m_fileToOpen.fileName(),
                                                 QSize(this->viewport()->width(),m_headinRectSize.height()),
                                                 QPoint(paddingX, yOffs));
//    itm->setTextColor(QColor(255,255,255, 255));
    itm->setSelectable(false);

    m_scene.addItem(itm);

    yOffs += itm->boundingRect().height() + paddingY;

    QGraphicsRectItem* separator = genSeparator(yOffs);
    m_scene.addItem(separator);

    yOffs += m_separatorHeight*2;

    for(int i=0; i < m_iconPaths.size(); ++i)
    {
        paintHandler(i, yOffs);
        yOffs += m_rowHeight + paddingY;
    }
}

QGraphicsRectItem* OpenWithGraphicsView::genSeparator(int yOffs)
{
    QRectF rct(0,yOffs, this->viewport()->width(), m_separatorHeight);
    QGraphicsRectItem* separator = new QGraphicsRectItem(rct);
    separator->setBrush(QBrush(QColor(0,0,0, 255)));
    return separator;
}

void OpenWithGraphicsView::paintHandler(int i, int yOffs)
{
    GraphicItemsBD::IconAndLabelItem* itm =
            new GraphicItemsBD::IconAndLabelItem(m_iconPaths[i], false,
                                                 m_programNames[i],
                                                 QSize(this->viewport()->width(),m_rowHeight),
                                                 QPoint(0, yOffs));

    m_scene.addItem(itm);
    items.push_back( itm );

    itm->setOnClicked([=](){
        deselectAll();
        itm->setSelected(true);
        m_selectedItem = i;
        emit clicked(i);
    });
}

void OpenWithGraphicsView::deselectAll()
{
    for(int i=0; i < items.size(); ++i)
    {
        items[i]->setSelected(false);
    }
}
