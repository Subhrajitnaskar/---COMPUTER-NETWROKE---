#include "my_label.h"

my_label::my_label(QWidget *parent) : QLabel(parent)
{
}

void my_label::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        QPoint pos = ev->pos();
        emit sendMouseClick(pos);
    }
}
