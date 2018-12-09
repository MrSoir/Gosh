#include "custommainwindowbd.h"

CustomMainWindowBD::CustomMainWindowBD(QWidget *parent)
    :QMainWindow(parent)
{
    this->statusBar()->hide();
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
}
void CustomMainWindowBD::makeTight(){
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
}
void CustomMainWindowBD::setLayout(QLayout* layout){
    QMainWindow::setLayout(layout);
    makeTight();
}
