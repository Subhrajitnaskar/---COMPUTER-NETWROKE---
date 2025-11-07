#ifndef MY_LABEL_H
#define MY_LABEL_H

#include <QLabel>
#include <QMouseEvent>

class my_label : public QLabel
{
    Q_OBJECT
public:
    explicit my_label(QWidget *parent = nullptr);

signals:
    void sendMouseClick(QPoint&);

protected:
    void mousePressEvent(QMouseEvent *ev) override;

};

#endif
