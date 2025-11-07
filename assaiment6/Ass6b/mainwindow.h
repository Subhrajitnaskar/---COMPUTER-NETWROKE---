#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPoint>
#include <QList>
#include <QPair>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMouseClick(QPoint& pos);
    void on_btnDrawGrid_clicked();
    void on_spinGridSize_valueChanged(int arg1);
    void on_btnDrawPolygon_clicked();
    void on_btnSetLine_clicked();
    void on_btnSetClipWindow_clicked();
    void on_btnCohenSutherland_clicked();
    void on_btnSutherlandHodgeman_clicked();
    void on_btnLiangBarsky_clicked();
    void on_btnWeilerAtherton_clicked();
    void on_btnResetClip_clicked();

private:
    Ui::MainWindow *ui;

    int gridSize = 20;
    int gridCenterX = 0;
    int gridCenterY = 0;

    QVector<QPoint> clickedPoints;
    QVector<QPoint> polygonVertices;

    QVector<QPair<QPoint, QPoint>> lines;

    QPoint windowMin, windowMax;
    bool isWindowSet = false;

    QColor polygonColor = Qt::blue;

    void drawGrid(bool clearWindow = true);
    void plotPixel(int x, int y, const QColor &color);
    void drawBresenhamLine(int x1, int y1, int x2, int y2, const QColor &color);
    void drawClipWindowFrame();
    QPoint screenToGrid(const QPoint& screenPos);

    typedef int OutCode;
    const OutCode INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8;
    OutCode computeOutCode(float x, float y) const;

    QVector<QPointF> clipPolygonSH(const QVector<QPointF>& subjectPolygon, int edge) const;
    bool isInsideSH(const QPointF& p, int edge) const;
    QPointF intersectSH(const QPointF& p1, const QPointF& p2, int edge) const;

    bool liangBarskyClipTest(float p, float q, float &t_enter, float &t_exit);

    bool clipLineCohenSutherland(const QPointF& p1, const QPointF& p2, QPointF& c1, QPointF& c2);
    bool clipLineLiangBarsky(const QPointF& p1, const QPointF& p2, QPointF& c1, QPointF& c2);

    struct VertexData {
        QPointF pos;
        bool isIntersection = false;
        bool isStartNode = false;
        int link = -1;
        bool visited = false;
    };
    QPointF getIntersectionWA(const QPointF& p1, const QPointF& p2, const QPointF& w1, const QPointF& w2);
    void weilerAthertonPolygonClip();
    bool isInsideClipWindow(const QPointF& p) const;

    bool pointInPolygon(const QVector<QPointF>& poly, const QPointF& test) const;

    QVector<QPoint> preClipPolygon;
    QVector<QPair<QPoint, QPoint>> preClipLines;
    bool hasPreClip = false;
};
#endif
