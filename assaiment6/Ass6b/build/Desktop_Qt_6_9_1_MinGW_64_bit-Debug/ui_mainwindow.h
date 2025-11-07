/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "my_label.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    my_label *frame;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupGrid;
    QGridLayout *gridLayout;
    QLabel *label;
    QSpinBox *spinGridSize;
    QPushButton *btnDrawGrid;
    QGroupBox *groupDrawing;
    QVBoxLayout *verticalLayout_2;
    QLabel *lblInfo;
    QPushButton *btnDrawPolygon;
    QPushButton *btnSetLine;
    QPushButton *btnSetClipWindow;
    QGroupBox *groupClipping;
    QVBoxLayout *verticalLayout_3;
    QPushButton *btnCohenSutherland;
    QPushButton *btnLiangBarsky;
    QPushButton *btnSutherlandHodgeman;
    QPushButton *btnWeilerAtherton;
    QPushButton *btnResetClip;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(888, 604);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        frame = new my_label(centralwidget);
        frame->setObjectName("frame");
        frame->setMinimumSize(QSize(600, 500));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Sunken);

        horizontalLayout->addWidget(frame);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        groupGrid = new QGroupBox(centralwidget);
        groupGrid->setObjectName("groupGrid");
        gridLayout = new QGridLayout(groupGrid);
        gridLayout->setObjectName("gridLayout");
        label = new QLabel(groupGrid);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        spinGridSize = new QSpinBox(groupGrid);
        spinGridSize->setObjectName("spinGridSize");
        spinGridSize->setMinimum(5);
        spinGridSize->setMaximum(50);
        spinGridSize->setValue(20);

        gridLayout->addWidget(spinGridSize, 0, 1, 1, 1);

        btnDrawGrid = new QPushButton(groupGrid);
        btnDrawGrid->setObjectName("btnDrawGrid");

        gridLayout->addWidget(btnDrawGrid, 1, 0, 1, 2);


        verticalLayout->addWidget(groupGrid);

        groupDrawing = new QGroupBox(centralwidget);
        groupDrawing->setObjectName("groupDrawing");
        verticalLayout_2 = new QVBoxLayout(groupDrawing);
        verticalLayout_2->setObjectName("verticalLayout_2");
        lblInfo = new QLabel(groupDrawing);
        lblInfo->setObjectName("lblInfo");
        lblInfo->setWordWrap(true);

        verticalLayout_2->addWidget(lblInfo);

        btnDrawPolygon = new QPushButton(groupDrawing);
        btnDrawPolygon->setObjectName("btnDrawPolygon");

        verticalLayout_2->addWidget(btnDrawPolygon);

        btnSetLine = new QPushButton(groupDrawing);
        btnSetLine->setObjectName("btnSetLine");

        verticalLayout_2->addWidget(btnSetLine);

        btnSetClipWindow = new QPushButton(groupDrawing);
        btnSetClipWindow->setObjectName("btnSetClipWindow");

        verticalLayout_2->addWidget(btnSetClipWindow);


        verticalLayout->addWidget(groupDrawing);

        groupClipping = new QGroupBox(centralwidget);
        groupClipping->setObjectName("groupClipping");
        verticalLayout_3 = new QVBoxLayout(groupClipping);
        verticalLayout_3->setObjectName("verticalLayout_3");
        btnCohenSutherland = new QPushButton(groupClipping);
        btnCohenSutherland->setObjectName("btnCohenSutherland");

        verticalLayout_3->addWidget(btnCohenSutherland);

        btnLiangBarsky = new QPushButton(groupClipping);
        btnLiangBarsky->setObjectName("btnLiangBarsky");

        verticalLayout_3->addWidget(btnLiangBarsky);

        btnSutherlandHodgeman = new QPushButton(groupClipping);
        btnSutherlandHodgeman->setObjectName("btnSutherlandHodgeman");

        verticalLayout_3->addWidget(btnSutherlandHodgeman);

        btnWeilerAtherton = new QPushButton(groupClipping);
        btnWeilerAtherton->setObjectName("btnWeilerAtherton");

        verticalLayout_3->addWidget(btnWeilerAtherton);


        verticalLayout->addWidget(groupClipping);

        btnResetClip = new QPushButton(centralwidget);
        btnResetClip->setObjectName("btnResetClip");

        verticalLayout->addWidget(btnResetClip);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout->addLayout(verticalLayout);

        horizontalLayout->setStretch(0, 3);
        horizontalLayout->setStretch(1, 1);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Clipping Algorithms", nullptr));
        frame->setText(QString());
        groupGrid->setTitle(QCoreApplication::translate("MainWindow", "Grid", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Size", nullptr));
        btnDrawGrid->setText(QCoreApplication::translate("MainWindow", "Draw/Reset Grid", nullptr));
        groupDrawing->setTitle(QCoreApplication::translate("MainWindow", "Drawing", nullptr));
        lblInfo->setText(QCoreApplication::translate("MainWindow", "Click on the grid to add points.", nullptr));
        btnDrawPolygon->setText(QCoreApplication::translate("MainWindow", "Draw Polygon (from clicks)", nullptr));
        btnSetLine->setText(QCoreApplication::translate("MainWindow", "Set Line (last 2 clicks)", nullptr));
        btnSetClipWindow->setText(QCoreApplication::translate("MainWindow", "Set Clip Window (last 2 clicks)", nullptr));
        groupClipping->setTitle(QCoreApplication::translate("MainWindow", "Clipping Algorithms", nullptr));
        btnCohenSutherland->setText(QCoreApplication::translate("MainWindow", "Cohen-Sutherland Clip Line", nullptr));
        btnLiangBarsky->setText(QCoreApplication::translate("MainWindow", "Liang-Barsky Clip Line", nullptr));
        btnSutherlandHodgeman->setText(QCoreApplication::translate("MainWindow", "Sutherland-Hodgeman Clip Poly", nullptr));
        btnWeilerAtherton->setText(QCoreApplication::translate("MainWindow", "Weiler-Atherton Clip Poly", nullptr));
        btnResetClip->setText(QCoreApplication::translate("MainWindow", "Reset Clip", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
