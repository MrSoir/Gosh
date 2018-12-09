#include "removewindowdialog.h"

RemoveWindowDialog::RemoveWindowDialog(int anzRects,
           const std::function<void(int)>& caller,
           Orientation::ORIENTATION orientation,
           QSize size,
           QPoint position,
           QWidget *parent)
    : m_anzRects(anzRects),
      caller(caller),
      QDialog(parent)
{
    setSize(size);
    setPosition(position);

    if(orientation == Orientation::ORIENTATION::HORIZONTAL){
        m_horizontal = true;
    }else{
        m_horizontal = false;
    }
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    setPosition(position);

    for(int i=0; i < 4; ++i)
        rects.append(new QRect());

    setMouseTracking(true);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    QPushButton* closeBtn = new QPushButton("cancel");
    QObject::connect(closeBtn, &QPushButton::clicked, [=](){
        this->close();
        this->deleteLater();
    });

    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->setAlignment(Qt::AlignBottom);
    vBox->addWidget(closeBtn);
    vBox->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vBox);
}

RemoveWindowDialog::~RemoveWindowDialog()
{
    caller = nullptr;
    for(int i=0; i < rects.size(); i++){
        delete rects[i];
    }
    rects.clear();
}

QRectF RemoveWindowDialog::boundingRect()
{
    return QRectF(0,0, width(), height());
}

void RemoveWindowDialog::setSize(const QSize &size)
{
    setMinimumSize(size);
    setMaximumSize(size);
}


void RemoveWindowDialog::setPosition(const QPoint &position)
{
    this->move(position.x(), position.y());
    repaint();
}

void RemoveWindowDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter* painter = new QPainter(this);

    // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
    QRectF rct = boundingRect();

    painter->setRenderHint(QPainter::Antialiasing, true);

//    painter->setClipRect(rct);

    QLinearGradient gradient(rct.topLeft(), rct.bottomRight());
    gradient.setColorAt(0, QColor(255,255,255, 255));
    gradient.setColorAt(1, QColor(220,220,255, 255));
    painter->fillRect(rct, gradient);

    QVector<QPoint> pos;
    QSize rectSize;

    if(m_anzRects == 1){
        if(!rects[0]){
            float fctr = 0.8;
            float wndwWdth = (float)rct.width()*fctr;
            float xStart = rct.x() + (rct.width()-wndwWdth)*0.5;
            float yStart = rct.y() + (rct.height()-wndwWdth)*0.5;
            QRect* wndwRct = new QRect(xStart,yStart, wndwWdth,wndwWdth);

            rects[0] = wndwRct;
        }
    }else if(m_anzRects == 2){
        float fctr = 0.9;
        float rctWidth = ((float)rct.width()) *0.4;
        float offs = ((float)rct.width())*(1.0-fctr)*0.5;

        if(m_horizontal){
            painter->drawLine(rct.left()+offs,
                              rct.center().y(),
                              rct.right()-offs,
                              rct.center().y());

            QSize size(rctWidth, rctWidth);
            QPoint p1, p2;
            p1 = QPoint(rct.center().x()-rctWidth*0.5,
                      rct.top()+offs);
            p2 = QPoint(rct.center().x()-rctWidth*0.5,
                      rct.bottom()-offs-rctWidth);

            pos.append(p1);
            pos.append(p2);
            rectSize = size;
        }else{
            painter->drawLine(rct.center().x(),
                              rct.top()+offs,
                              rct.center().x(),
                              rct.bottom()-offs);

            QSize size(rctWidth, rctWidth);
            QPoint p1, p2;
            p1 = QPoint(rct.left()+offs,
                      rct.center().y()-rctWidth*0.5);
            p2 = QPoint(rct.right()-offs-rctWidth,
                      rct.center().y()-rctWidth*0.5);

            pos.append(p1);
            pos.append(p2);
            rectSize = size;
        }
    }else if(m_anzRects == 3){
        float fctr = 0.9;
        float rctWidth = ((float)rct.width()) *0.4;
        float offs = ((float)rct.width())*(1.0-fctr)*0.5;
        if(m_horizontal){
            painter->drawLine(rct.left()+offs,
                              rct.center().y(),
                              rct.right()-offs,
                              rct.center().y());
            painter->drawLine(rct.center().x(),
                              rct.center().y(),
                              rct.center().x(),
                              rct.bottom()-offs);

            QSize size(rctWidth, rctWidth);
            QPoint p1, p2, p3;
            p1 = QPoint(rct.center().x()-rctWidth*0.5,
                      rct.top()+offs);
            p2 = QPoint(rct.left()+offs,
                      rct.bottom()-offs-rctWidth);
            p3 = QPoint(rct.right()-offs-rctWidth,
                      rct.bottom()-offs-rctWidth);

            pos.append(p1);
            pos.append(p2);
            pos.append(p3);
            rectSize = size;
        }else{
            painter->drawLine(rct.center().x(),
                              rct.top()+offs,
                              rct.center().x(),
                              rct.bottom()-offs);
            painter->drawLine(rct.center().x(),
                              rct.center().y(),
                              rct.right()-offs,
                              rct.center().y());

            QSize size(rctWidth, rctWidth);
            QPoint p1, p2, p3;
            p1 = QPoint(rct.left()+offs,
                      rct.center().y()-rctWidth*0.5);
            p2 = QPoint(rct.right()-offs-rctWidth,
                      rct.top()+offs);
            p3 = QPoint(rct.right()-offs-rctWidth,
                      rct.bottom()-offs-rctWidth);

            pos.append(p1);
            pos.append(p2);
            pos.append(p3);
            rectSize = size;
        }
    }else{ // == 4
        float fctr = 0.9;
        float rctWidth = ((float)rct.width()) *0.4;
        float offs = ((float)rct.width())*(1.0-fctr)*0.5;

        painter->drawLine(rct.left()+offs,
                          rct.center().y(),
                          rct.right()-offs,
                          rct.center().y());
        painter->drawLine(rct.center().x(),
                          rct.top()+offs,
                          rct.center().x(),
                          rct.bottom()-offs);
        QSize size(rctWidth, rctWidth);
        QPoint p1, p2, p3, p4;
        p1 = QPoint(rct.left()+offs,
                  rct.top()+offs);
        p2 = QPoint(rct.right()-offs-rctWidth,
                  rct.top()+offs);
        p3 = QPoint(rct.left()+offs,
                  rct.bottom()-offs-rctWidth);
        p4 = QPoint(rct.right()-offs-rctWidth,
                  rct.bottom()-offs-rctWidth);

        pos.append(p1);
        pos.append(p3);
        pos.append(p2);
        pos.append(p4);
        rectSize = size;
    }

    for(int i=0; i < pos.size(); ++i)
    {
        setRectsPosition(rects[i], pos[i]);
        rects[i]->setSize(rectSize);
    }

    for(int i=0; i < m_anzRects; i++){
        StaticFunctions::paintRect(painter, *rects[i]);
    }
    if(mouseRectId > -1 && rects[mouseRectId]){
        StaticFunctions::paintCrossRect(painter, *rects[mouseRectId]);
    }
//    QDialog::paintEvent(event);
}

void RemoveWindowDialog::mousePressEvent(QMouseEvent *event){
    QPointF mouP = event->pos();
    for(int i=0; i < m_anzRects; i++){
        if(rects[i] && caller
                && rects[i]->contains(QPoint(mouP.x(), mouP.y()))){
            caller(i);
            this->close();
            this->deleteLater();
        }
    }
    qint64 curTime = QDateTime::currentMSecsSinceEpoch();
    if(curTime - lastTmePrsd < 300){

    }
    lastTmePrsd = curTime;
    isPressed = true;
    repaint();
    //        update();
}

void RemoveWindowDialog::mouseMoveEvent(QMouseEvent *event){
    m_hover = true;
    QPointF mouP = event->pos();
    mouseRectId = -1;
    for(int i=0; i < m_anzRects; i++){
        if(rects[i] && rects[i]->contains(QPoint(mouP.x(), mouP.y()))){
            mouseRectId = i;
            break;
        }
    }
//        update();
    repaint();
    return QWidget::mouseMoveEvent(event);
}

void RemoveWindowDialog::setRectsPosition(QRect *rect, const QPoint &pos)
{
    rect->setX(pos.x());
    rect->setY(pos.y());
}

