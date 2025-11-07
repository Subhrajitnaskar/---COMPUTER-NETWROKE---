#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QVector>
#include <QList>
#include <cmath>
#include <algorithm>
#include <QLineF>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->frame, &my_label::sendMouseClick, this, &MainWindow::onMouseClick);
    gridSize = ui->spinGridSize->value();
    drawGrid(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawGrid(bool clearWindow) {
    QPixmap canvas(ui->frame->size());
    canvas.fill(Qt::white);
    QPainter painter(&canvas);

    gridCenterX = canvas.width() / 2;
    gridCenterY = canvas.height() / 2;

    QPen gridPen(Qt::lightGray);
    painter.setPen(gridPen);
    for (int x = gridCenterX; x < canvas.width(); x += gridSize) painter.drawLine(x, 0, x, canvas.height());
    for (int x = gridCenterX; x > 0; x -= gridSize) painter.drawLine(x, 0, x, canvas.height());
    for (int y = gridCenterY; y < canvas.height(); y += gridSize) painter.drawLine(0, y, canvas.width(), y);
    for (int y = gridCenterY; y > 0; y -= gridSize) painter.drawLine(0, y, canvas.width(), y);

    ui->frame->setPixmap(canvas);

    int xRange = gridCenterX / gridSize;
    int yRange = gridCenterY / gridSize;
    for(int i = -xRange; i <= xRange; ++i) plotPixel(i, 0, Qt::black);
    for(int i = -yRange; i <= yRange; ++i) plotPixel(0, i, Qt::black);

    clickedPoints.clear();
    polygonVertices.clear();
    lines.clear();

    if (clearWindow) {
        isWindowSet = false;
    }

    ui->lblInfo->setText("Grid reset. Click to add points.");
}

QPoint MainWindow::screenToGrid(const QPoint &screenPos) {
    int gridX = floor((float)(screenPos.x() - gridCenterX) / gridSize);
    int gridY = floor((float)(gridCenterY - screenPos.y()) / gridSize);
    return QPoint(gridX, gridY);
}

void MainWindow::plotPixel(int x, int y, const QColor &color) {
    QPixmap canvas = ui->frame->pixmap(Qt::ReturnByValue);
    QPainter painter(&canvas);
    painter.setPen(color);
    painter.setBrush(color);
    int screenTopLeftX = gridCenterX + x * gridSize;
    int screenTopLeftY = gridCenterY - (y + 1) * gridSize;
    painter.drawRect(screenTopLeftX, screenTopLeftY, gridSize, gridSize);
    ui->frame->setPixmap(canvas);
}

void MainWindow::drawBresenhamLine(int x1, int y1, int x2, int y2, const QColor &color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        plotPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void MainWindow::drawClipWindowFrame() {
    if (!isWindowSet) return;
    drawBresenhamLine(windowMin.x(), windowMin.y(), windowMax.x(), windowMin.y(), Qt::red);
    drawBresenhamLine(windowMax.x(), windowMin.y(), windowMax.x(), windowMax.y(), Qt::red);
    drawBresenhamLine(windowMax.x(), windowMax.y(), windowMin.x(), windowMax.y(), Qt::red);
    drawBresenhamLine(windowMin.x(), windowMax.y(), windowMin.x(), windowMin.y(), Qt::red);
}

void MainWindow::onMouseClick(QPoint &pos) {
    QPoint gridPos = screenToGrid(pos);
    clickedPoints.append(gridPos);
    plotPixel(gridPos.x(), gridPos.y(), Qt::magenta);
    ui->lblInfo->setText(QString("Added point (%1, %2). Total clicks: %3")
                             .arg(gridPos.x()).arg(gridPos.y()).arg(clickedPoints.size()));
}

void MainWindow::on_btnDrawGrid_clicked() {
    drawGrid(true);
}

void MainWindow::on_spinGridSize_valueChanged(int val) {
    gridSize = val;
    drawGrid(true);
}

void MainWindow::on_btnDrawPolygon_clicked() {
    if (clickedPoints.size() < 3) {
        ui->lblInfo->setText("Need at least 3 points for a polygon.");
        return;
    }
    polygonVertices = clickedPoints;
    for (int i = 0; i < polygonVertices.size(); ++i) {
        QPoint p1 = polygonVertices[i];
        QPoint p2 = polygonVertices[(i + 1) % polygonVertices.size()];
        drawBresenhamLine(p1.x(), p1.y(), p2.x(), p2.y(), polygonColor);
    }
    clickedPoints.clear();
    ui->lblInfo->setText("Polygon drawn. Ready to clip.");
}

void MainWindow::on_btnSetLine_clicked() {
    if (clickedPoints.size() < 2) {
        ui->lblInfo->setText("Need at least 2 points for a line.");
        return;
    }
    QPoint p1 = clickedPoints[clickedPoints.size()-2];
    QPoint p2 = clickedPoints.back();
    lines.append(qMakePair(p1, p2));

    drawBresenhamLine(p1.x(), p1.y(), p2.x(), p2.y(), Qt::darkGreen);

    clickedPoints.clear();
    ui->lblInfo->setText(QString("Line added. Total lines: %1. Ready to clip.").arg(lines.size()));
}

void MainWindow::on_btnSetClipWindow_clicked() {
    if (clickedPoints.size() < 2) {
        ui->lblInfo->setText("Need 2 corner points for a window.");
        return;
    }
    QPoint p1 = clickedPoints[clickedPoints.size()-2];
    QPoint p2 = clickedPoints.back();
    windowMin.setX(qMin(p1.x(), p2.x()));
    windowMin.setY(qMin(p1.y(), p2.y()));
    windowMax.setX(qMax(p1.x(), p2.x()));
    windowMax.setY(qMax(p1.y(), p2.y()));
    isWindowSet = true;
    drawClipWindowFrame();
    clickedPoints.clear();
    ui->lblInfo->setText("Clip window set.");
}

MainWindow::OutCode MainWindow::computeOutCode(float x, float y) const {
    OutCode code = INSIDE;
    if (x < windowMin.x()) code = (OutCode)(code | LEFT);
    else if (x > windowMax.x()) code = (OutCode)(code | RIGHT);
    if (y < windowMin.y()) code = (OutCode)(code | BOTTOM);
    else if (y > windowMax.y()) code = (OutCode)(code | TOP);
    return code;
}

bool MainWindow::clipLineCohenSutherland(const QPointF& p1, const QPointF& p2, QPointF& c1, QPointF& c2) {
    float x1 = p1.x(), y1 = p1.y();
    float x2 = p2.x(), y2 = p2.y();
    OutCode outcode1 = computeOutCode(x1, y1);
    OutCode outcode2 = computeOutCode(x2, y2);
    bool accept = false;

    while (true) {
        if (!(outcode1 | outcode2)) { accept = true; break; }
        else if (outcode1 & outcode2) { break; }
        else {
            float x=0, y=0;
            OutCode outcodeOut = outcode1 ? outcode1 : outcode2;
            if (outcodeOut & TOP) {
                if (y2 - y1 != 0) x = x1 + (x2 - x1) * (windowMax.y() - y1) / (y2 - y1); else x = x1;
                y = windowMax.y();
            } else if (outcodeOut & BOTTOM) {
                if (y2 - y1 != 0) x = x1 + (x2 - x1) * (windowMin.y() - y1) / (y2 - y1); else x = x1;
                y = windowMin.y();
            } else if (outcodeOut & RIGHT) {
                if (x2 - x1 != 0) y = y1 + (y2 - y1) * (windowMax.x() - x1) / (x2 - x1); else y = y1;
                x = windowMax.x();
            } else {
                if (x2 - x1 != 0) y = y1 + (y2 - y1) * (windowMin.x() - x1) / (x2 - x1); else y = y1;
                x = windowMin.x();
            }
            if (outcodeOut == outcode1) { x1 = x; y1 = y; outcode1 = computeOutCode(x1, y1); }
            else { x2 = x; y2 = y; outcode2 = computeOutCode(x2, y2); }
        }
    }
    if (accept) { c1 = QPointF(x1, y1); c2 = QPointF(x2, y2); }
    return accept;
}

void MainWindow::on_btnCohenSutherland_clicked() {
    if (lines.isEmpty() || !isWindowSet) {
        ui->lblInfo->setText("Add at least one line and set a clip window first!");
        return;
    }

    preClipLines = lines;
    preClipPolygon = polygonVertices;
    hasPreClip = true;

    auto linesCopy = lines;

    drawGrid(false);
    drawClipWindowFrame();

    for (const auto& seg : linesCopy) {
        drawBresenhamLine(seg.first.x(), seg.first.y(), seg.second.x(), seg.second.y(), Qt::lightGray);
    }

    int accepted = 0, rejected = 0;
    for (const auto& seg : linesCopy) {
        QPointF c1, c2;
        if (clipLineCohenSutherland(QPointF(seg.first), QPointF(seg.second), c1, c2)) {
            drawBresenhamLine(std::lround(c1.x()), std::lround(c1.y()),
                              std::lround(c2.x()), std::lround(c2.y()), Qt::darkGreen);
            ++accepted;
        } else {
            ++rejected;
        }
    }
    ui->lblInfo->setText(QString("Cohen-Sutherland: %1 clipped, %2 rejected.").arg(accepted).arg(rejected));
}

bool MainWindow::isInsideSH(const QPointF& p, int edge) const {
    switch (edge) {
    case 0: return p.x() >= windowMin.x();
    case 1: return p.y() <= windowMax.y();
    case 2: return p.x() <= windowMax.x();
    case 3: return p.y() >= windowMin.y();
    } return false;
}

QPointF MainWindow::intersectSH(const QPointF& p1, const QPointF& p2, int edge) const {
    float x=0, y=0; float dx = p2.x() - p1.x(); float dy = p2.y() - p1.y();
    switch(edge) {
    case 0: x = windowMin.x(); y = (dx != 0) ? (p1.y() + dy * (x - p1.x()) / dx) : p1.y(); break;
    case 1: y = windowMax.y(); x = (dy != 0) ? (p1.x() + dx * (y - p1.y()) / dy) : p1.x(); break;
    case 2: x = windowMax.x(); y = (dx != 0) ? (p1.y() + dy * (x - p1.x()) / dx) : p1.y(); break;
    case 3: y = windowMin.y(); x = (dy != 0) ? (p1.x() + dx * (y - p1.y()) / dy) : p1.x(); break;
    } return QPointF(x, y);
}

QVector<QPointF> MainWindow::clipPolygonSH(const QVector<QPointF> &subjectPolygon, int edge) const {
    QVector<QPointF> outputList; if (subjectPolygon.isEmpty()) return outputList;
    QPointF S = subjectPolygon.last();
    for (const QPointF& P : subjectPolygon) {
        if (isInsideSH(P, edge)) {
            if (!isInsideSH(S, edge)) { outputList.append(intersectSH(S, P, edge)); }
            outputList.append(P);
        } else if (isInsideSH(S, edge)) { outputList.append(intersectSH(S, P, edge)); }
        S = P;
    } return outputList;
}

void MainWindow::on_btnSutherlandHodgeman_clicked() {
    if (polygonVertices.size() < 3 || !isWindowSet) {
        ui->lblInfo->setText("Draw a polygon and set a clip window first!"); return;
    }

    preClipLines = lines;
    preClipPolygon = polygonVertices;
    hasPreClip = true;

    QVector<QPoint> origPoly = polygonVertices;

    QVector<QPointF> subjectPolygonF;
    for(const QPoint& p : polygonVertices) subjectPolygonF.append(QPointF(p));
    QVector<QPointF> clipped = subjectPolygonF;
    clipped = clipPolygonSH(clipped, 0); clipped = clipPolygonSH(clipped, 1);
    clipped = clipPolygonSH(clipped, 2); clipped = clipPolygonSH(clipped, 3);

    drawGrid(false);
    drawClipWindowFrame();

    if (!origPoly.isEmpty()) {
        for (int i = 0; i < origPoly.size(); ++i) {
            QPoint p1 = origPoly[i];
            QPoint p2 = origPoly[(i + 1) % origPoly.size()];
            drawBresenhamLine(p1.x(), p1.y(), p2.x(), p2.y(), Qt::lightGray);
        }
    }

    if (!clipped.isEmpty()) {
        for (int i = 0; i < clipped.size(); ++i) {
            QPointF p1 = clipped[i]; QPointF p2 = clipped[(i + 1) % clipped.size()];
            drawBresenhamLine(round(p1.x()), round(p1.y()), round(p2.x()), round(p2.y()), polygonColor);
        }
        ui->lblInfo->setText("Polygon clipped with Sutherland-Hodgeman.");
    } else {
        ui->lblInfo->setText("Polygon is outside. Rejected.");
    }
}

bool MainWindow::liangBarskyClipTest(float p, float q, float &t_enter, float &t_exit) {
    if (abs(p) < 1e-6) {
        if (q < 0) return false;
    } else {
        float t = q / p;
        if (p < 0) {
            if (t > t_exit) return false;
            t_enter = qMax(t_enter, t);
        } else {
            if (t < t_enter) return false;
            t_exit = qMin(t_exit, t);
        }
    }
    return true;
}

bool MainWindow::clipLineLiangBarsky(const QPointF& p1, const QPointF& p2, QPointF& c1, QPointF& c2) {
    float x1 = p1.x(), y1 = p1.y();
    float x2 = p2.x(), y2 = p2.y();
    float t_enter = 0.0f, t_exit = 1.0f;
    float dx = x2 - x1, dy = y2 - y1;

    if (liangBarskyClipTest(-dx, x1 - windowMin.x(), t_enter, t_exit) &&
        liangBarskyClipTest(dx, windowMax.x() - x1, t_enter, t_exit) &&
        liangBarskyClipTest(-dy, y1 - windowMin.y(), t_enter, t_exit) &&
        liangBarskyClipTest(dy, windowMax.y() - y1, t_enter, t_exit))
    {
        if (t_exit < 1.0f) { x2 = x1 + t_exit * dx; y2 = y1 + t_exit * dy; }
        if (t_enter > 0.0f) { x1 = x1 + t_enter * dx; y1 = y1 + t_enter * dy; }
        c1 = QPointF(x1, y1);
        c2 = QPointF(x2, y2);
        return true;
    }
    return false;
}

void MainWindow::on_btnLiangBarsky_clicked() {
    if (lines.isEmpty() || !isWindowSet) {
        ui->lblInfo->setText("Add at least one line and set a clip window first!");
        return;
    }

    preClipLines = lines;
    preClipPolygon = polygonVertices;
    hasPreClip = true;

    auto linesCopy = lines;

    drawGrid(false);
    drawClipWindowFrame();

    for (const auto& seg : linesCopy) {
        drawBresenhamLine(seg.first.x(), seg.first.y(), seg.second.x(), seg.second.y(), Qt::lightGray);
    }

    int accepted = 0, rejected = 0;
    for (const auto& seg : linesCopy) {
        QPointF c1, c2;
        if (clipLineLiangBarsky(QPointF(seg.first), QPointF(seg.second), c1, c2)) {
            drawBresenhamLine(std::lround(c1.x()), std::lround(c1.y()),
                              std::lround(c2.x()), std::lround(c2.y()), Qt::darkCyan);
            ++accepted;
        } else {
            ++rejected;
        }
    }
    ui->lblInfo->setText(QString("Liang-Barsky: %1 clipped, %2 rejected.").arg(accepted).arg(rejected));
}

void MainWindow::on_btnWeilerAtherton_clicked() {
    if (polygonVertices.size() < 3 || !isWindowSet) {
        ui->lblInfo->setText("Draw a polygon and set a clip window first!"); return;
    }

    preClipLines = lines;
    preClipPolygon = polygonVertices;
    hasPreClip = true;

    weilerAthertonPolygonClip();
}

QPointF getIntersection(const QPointF& p1, const QPointF& p2, const QPointF& q1, const QPointF& q2) {
    double A1 = p2.y() - p1.y();
    double B1 = p1.x() - p2.x();
    double C1 = A1 * p1.x() + B1 * p1.y();
    double A2 = q2.y() - q1.y();
    double B2 = q1.x() - q2.x();
    double C2 = A2 * q1.x() + B2 * q1.y();
    double det = A1 * B2 - A2 * B1;
    if (std::abs(det) < 1e-9) return QPointF();
    double x = (B2 * C1 - B1 * C2) / det;
    double y = (A1 * C2 - A2 * C1) / det;
    const double eps = 1e-6;
    if (x >= std::min(p1.x(), p2.x()) - eps && x <= std::max(p1.x(), p2.x()) + eps &&
        y >= std::min(p1.y(), p2.y()) - eps && y <= std::max(p1.y(), p2.y()) + eps &&
        x >= std::min(q1.x(), q2.x()) - eps && x <= std::max(q1.x(), q2.x()) + eps &&
        y >= std::min(q1.y(), q2.y()) - eps && y <= std::max(q1.y(), q2.y()) + eps) {
        return QPointF(x, y);
    }
    return QPointF();
}

bool MainWindow::isInsideClipWindow(const QPointF& p) const {
    return p.x() >= windowMin.x() && p.x() <= windowMax.x() &&
           p.y() >= windowMin.y() && p.y() <= windowMax.y();
}

bool MainWindow::pointInPolygon(const QVector<QPointF>& poly, const QPointF& test) const {
    bool inside = false;
    int n = poly.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const QPointF& pi = poly[i];
        const QPointF& pj = poly[j];
        bool intersect = ((pi.y() > test.y()) != (pj.y() > test.y())) &&
                         (test.x() < (pj.x() - pi.x()) * (test.y() - pi.y()) / (pj.y() - pi.y() + 1e-12) + pi.x());
        if (intersect) inside = !inside;
    }
    return inside;
}

void MainWindow::weilerAthertonPolygonClip() {
    QVector<QPoint> origPolyInt = polygonVertices;
    QVector<QPointF> origPoly;
    for (const auto& q : origPolyInt) origPoly.append(QPointF(q));

    QList<VertexData> subjectList;
    for (const auto& p : polygonVertices) subjectList.append({QPointF(p)});

    QList<VertexData> clipList;
    QVector<QPoint> windowVerts = {windowMin, QPoint(windowMax.x(), windowMin.y()), windowMax, QPoint(windowMin.x(), windowMax.y())};
    for (const auto& p : windowVerts) clipList.append({QPointF(p)});

    bool allInside = true;
    for(const auto& v : subjectList) {
        if (!isInsideClipWindow(v.pos)) {
            allInside = false;
            break;
        }
    }

    QList<QPair<int, VertexData>> subjectInserts, clipInserts;
    for (int i = 0; i < subjectList.size(); ++i) {
        QPointF p1 = subjectList[i].pos;
        QPointF p2 = subjectList[(i + 1) % subjectList.size()].pos;

        for (int j = 0; j < clipList.size(); ++j) {
            QPointF w1 = clipList[j].pos;
            QPointF w2 = clipList[(j + 1) % clipList.size()].pos;

            QPointF intersectPt = getIntersection(p1, p2, w1, w2);
            if (!intersectPt.isNull()) {
                bool isStartNode = isInsideClipWindow(p1) && !isInsideClipWindow(p2);
                subjectInserts.append({i + 1, {intersectPt, true, isStartNode, -1, false}});
                clipInserts.append({j + 1, {intersectPt, true, isStartNode, -1, false}});
            }
        }
    }

    drawGrid(false);
    drawClipWindowFrame();
    if (!origPoly.isEmpty()) {
        for (int i = 0; i < origPoly.size(); ++i) {
            QPointF p1 = origPoly[i];
            QPointF p2 = origPoly[(i + 1) % origPoly.size()];
            drawBresenhamLine(round(p1.x()), round(p1.y()), round(p2.x()), round(p2.y()), Qt::lightGray);
        }
    }

    if (subjectInserts.isEmpty()) {
        if (allInside) {
            if (!origPoly.isEmpty()) {
                for (int i = 0; i < origPoly.size(); ++i) {
                    QPointF p1 = origPoly[i];
                    QPointF p2 = origPoly[(i + 1) % origPoly.size()];
                    drawBresenhamLine(round(p1.x()), round(p1.y()), round(p2.x()), round(p2.y()), polygonColor);
                }
            }
            ui->lblInfo->setText("Polygon is fully inside window.");
        } else {

            QPointF winCenter((windowMin.x() + windowMax.x()) / 2.0, (windowMin.y() + windowMax.y()) / 2.0);

            if (!origPoly.isEmpty() && pointInPolygon(origPoly, winCenter)) {
                drawBresenhamLine(windowMin.x(), windowMin.y(), windowMax.x(), windowMin.y(), polygonColor);
                drawBresenhamLine(windowMax.x(), windowMin.y(), windowMax.x(), windowMax.y(), polygonColor);
                drawBresenhamLine(windowMax.x(), windowMax.y(), windowMin.x(), windowMax.y(), polygonColor);
                drawBresenhamLine(windowMin.x(), windowMax.y(), windowMin.x(), windowMin.y(), polygonColor);
                ui->lblInfo->setText("Weiler-Atherton: Window lies inside polygon — colored with polygon color.");
            } else {
                ui->lblInfo->setText("Polygon is fully outside window.");
            }
        }
        return;
    }


    std::sort(subjectInserts.begin(), subjectInserts.end(), [](auto &a, auto &b) { return a.first > b.first; });
    for (auto &p : subjectInserts) subjectList.insert(p.first, p.second);

    std::sort(clipInserts.begin(), clipInserts.end(), [](auto &a, auto &b) { return a.first > b.first; });
    for (auto &p : clipInserts) clipList.insert(p.first, p.second);

    for (int i = 0; i < subjectList.size(); ++i) {
        if (subjectList[i].isIntersection) {
            for (int j = 0; j < clipList.size(); ++j) {
                if (clipList[j].isIntersection && subjectList[i].pos == clipList[j].pos) {
                    subjectList[i].link = j;
                    clipList[j].link = i;
                    break;
                }
            }
        }
    }

    QList<QVector<QPointF>> resultPolygons;
    for (int i = 0; i < subjectList.size(); ++i) {
        if (subjectList[i].isIntersection && subjectList[i].isStartNode && !subjectList[i].visited) {
            QVector<QPointF> currentPoly;
            int currentIdx = i;
            bool isSubjList = true;

            do {
                VertexData* v = isSubjList ? &subjectList[currentIdx] : &clipList[currentIdx];
                v->visited = true;
                currentPoly.append(v->pos);

                if (v->isIntersection && v->link != -1) {
                    isSubjList = !isSubjList;
                    currentIdx = v->link;
                }

                currentIdx = (currentIdx + 1) % (isSubjList ? subjectList.size() : clipList.size());

            } while (currentPoly.first() != currentPoly.back() || currentPoly.size() < 2);

            currentPoly.pop_back();
            if (currentPoly.size() >= 3) {
                resultPolygons.append(currentPoly);
            }
        }
    }

    if (!resultPolygons.isEmpty()) {
        for (const auto& poly : resultPolygons) {
            for (int i = 0; i < poly.size(); ++i) {
                QPointF p1 = poly[i];
                QPointF p2 = poly[(i + 1) % poly.size()];
                drawBresenhamLine(round(p1.x()), round(p1.y()), round(p2.x()), round(p2.y()), polygonColor);
            }
        }
        ui->lblInfo->setText("Polygon clipped with Weiler-Atherton.");
    } else {
        ui->lblInfo->setText("Polygon is outside or clipped to nothing.");
    }
}

void MainWindow::on_btnResetClip_clicked() {
    if (!hasPreClip) {
        ui->lblInfo->setText("Nothing to reset.");
        return;
    }

    drawGrid(false);
    drawClipWindowFrame();

    polygonVertices = preClipPolygon;
    lines = preClipLines;

    if (polygonVertices.size() >= 3) {
        for (int i = 0; i < polygonVertices.size(); ++i) {
            const QPoint& p1 = polygonVertices[i];
            const QPoint& p2 = polygonVertices[(i + 1) % polygonVertices.size()];
            drawBresenhamLine(p1.x(), p1.y(), p2.x(), p2.y(), polygonColor);
        }
    }

    for (const auto& seg : lines) {
        drawBresenhamLine(seg.first.x(), seg.first.y(), seg.second.x(), seg.second.y(), Qt::darkGreen);
    }

    ui->lblInfo->setText("Restored shapes to the state before last clip.");
}
