/*
    Copyright 2016-2026 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include "CustomTitleBar.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QWindow>
#include <QApplication>
#include <QScreen>
#include <QtGlobal>

// ------------------------------------------------------------------
// TitleBarLogo
// ------------------------------------------------------------------

TitleBarLogo::TitleBarLogo(QWidget* parent) : QWidget(parent)
{
    setFixedSize(22, 22);
}

void TitleBarLogo::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF r = rect().adjusted(1, 1, -1, -1);

    QLinearGradient grad(r.topLeft(), r.bottomRight());
    grad.setColorAt(0.0, QColor(0x3d, 0x5a, 0xfe));  // blue
    grad.setColorAt(1.0, QColor(0x22, 0xe0, 0xd0));  // turquoise

    QPainterPath path;
    path.addRoundedRect(r, 6, 6);

    p.fillPath(path, grad);

    // inner facet line to make it read as a little emblem rather than a flat swatch
    QPen pen(QColor(255, 255, 255, 90), 1.2);
    p.setPen(pen);
    p.drawLine(QPointF(r.left() + r.width() * 0.32, r.top() + 2),
               QPointF(r.left() + r.width() * 0.32, r.bottom() - 2));
}

// ------------------------------------------------------------------
// TitleBarButton
// ------------------------------------------------------------------

TitleBarButton::TitleBarButton(const QString& glyphPath, QWidget* parent)
    : QToolButton(parent), glyph(glyphPath)
{
    setFixedSize(38, 30);
    setCursor(Qt::ArrowCursor);
    setAttribute(Qt::WA_Hover, true);
}

void TitleBarButton::mousePressEvent(QMouseEvent* event)
{
    pressedDown = true;
    update();
    QToolButton::mousePressEvent(event);
}

void TitleBarButton::mouseReleaseEvent(QMouseEvent* event)
{
    pressedDown = false;
    update();
    QToolButton::mouseReleaseEvent(event);
}

void TitleBarButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF r = rect();
    // "pressed" look: the whole glyph nudges down 1px and the glow softens
    QPointF pressOffset = pressedDown ? QPointF(0, 1) : QPointF(0, 0);

    if (underMouse())
    {
        QColor glow = hoverColor;
        glow.setAlpha(pressedDown ? 60 : 110);
        p.fillRect(r.adjusted(4, 3, -4, -3), glow);
    }

    QPen pen(pressedDown ? hoverColor.lighter(115) : QColor(0xcf, 0xd3, 0xdc), 1.6);
    p.setPen(pen);

    QPointF c = r.center() + pressOffset;
    const qreal s = 5.0; // glyph half-size

    if (glyph == "min")
    {
        p.drawLine(QPointF(c.x() - s, c.y()), QPointF(c.x() + s, c.y()));
    }
    else if (glyph == "max")
    {
        p.drawRect(QRectF(c.x() - s, c.y() - s, s * 2, s * 2));
    }
    else if (glyph == "restore")
    {
        p.drawRect(QRectF(c.x() - s + 2.5, c.y() - s, s * 2 - 2.5, s * 2 - 2.5));
        p.drawRect(QRectF(c.x() - s, c.y() - s + 2.5, s * 2 - 2.5, s * 2 - 2.5));
    }
    else if (glyph == "close")
    {
        p.drawLine(QPointF(c.x() - s, c.y() - s), QPointF(c.x() + s, c.y() + s));
        p.drawLine(QPointF(c.x() - s, c.y() + s), QPointF(c.x() + s, c.y() - s));
    }
}

// ------------------------------------------------------------------
// CustomTitleBar
// ------------------------------------------------------------------

CustomTitleBar::CustomTitleBar(QWidget* window, QWidget* parent)
    : QWidget(parent), hostWindow(window)
{
    setFixedHeight(34);
    setObjectName("customTitleBar");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 6, 0);
    layout->setSpacing(8);

    auto* logo = new TitleBarLogo(this);
    layout->addWidget(logo, 0, Qt::AlignVCenter);

    titleLabel = new QLabel(hostWindow->windowTitle(), this);
    titleLabel->setObjectName("titleBarLabel");
    layout->addWidget(titleLabel, 0, Qt::AlignVCenter);

    layout->addStretch(1);

    btnMin = new TitleBarButton("min", this);
    btnMin->setObjectName("titleBarButton");
    btnMax = new TitleBarButton("max", this);
    btnMax->setObjectName("titleBarButton");
    btnClose = new TitleBarButton("close", this);
    btnClose->setObjectName("titleBarCloseButton");
    btnClose->setHoverColor(QColor(0xe0, 0x3d, 0x4a));

    layout->addWidget(btnMin);
    layout->addWidget(btnMax);
    layout->addWidget(btnClose);

    connect(btnMin, &QToolButton::clicked, this, [this]() { hostWindow->showMinimized(); });
    connect(btnMax, &QToolButton::clicked, this, [this]()
    {
        if (hostWindow->isMaximized())
            hostWindow->showNormal();
        else
            hostWindow->showMaximized();
        refreshMaximizeGlyph();
    });
    connect(btnClose, &QToolButton::clicked, this, [this]() { hostWindow->close(); });

    connect(hostWindow, &QWidget::windowTitleChanged, this, &CustomTitleBar::setTitleText);
}

void CustomTitleBar::setTitleText(const QString& title)
{
    titleLabel->setText(title);
}

void CustomTitleBar::refreshMaximizeGlyph()
{
    btnMax->setGlyph(hostWindow->isMaximized() ? "restore" : "max");
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    if (hostWindow->windowHandle())
    {
        hostWindow->windowHandle()->startSystemMove();
        return;
    }
#endif
    // Fallback for Qt < 5.15, which lacks QWindow::startSystemMove().
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    dragOrigin = event->globalPosition().toPoint() - hostWindow->pos();
#else
    dragOrigin = event->globalPos() - hostWindow->pos();
#endif
    dragging = true;
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* event)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    if (dragging && (event->buttons() & Qt::LeftButton))
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        hostWindow->move(event->globalPosition().toPoint() - dragOrigin);
#else
        hostWindow->move(event->globalPos() - dragOrigin);
#endif
    }
#else
    Q_UNUSED(event);
#endif
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent*)
{
    dragging = false;
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent*)
{
    if (hostWindow->isMaximized())
        hostWindow->showNormal();
    else
        hostWindow->showMaximized();
    refreshMaximizeGlyph();
}

void CustomTitleBar::paintEvent(QPaintEvent*)
{
    // background is handled via QSS (#customTitleBar); nothing extra to draw
}

// ------------------------------------------------------------------
// WindowResizeGrips
// ------------------------------------------------------------------

namespace
{
    // Order matches WindowResizeGrips::grips[]
    enum GripIndex { GripN, GripNE, GripE, GripSE, GripS, GripSW, GripW, GripNW };

    Qt::Edges EdgesForGrip(int idx)
    {
        switch (idx)
        {
            case GripN:  return Qt::TopEdge;
            case GripNE: return Qt::TopEdge | Qt::RightEdge;
            case GripE:  return Qt::RightEdge;
            case GripSE: return Qt::BottomEdge | Qt::RightEdge;
            case GripS:  return Qt::BottomEdge;
            case GripSW: return Qt::BottomEdge | Qt::LeftEdge;
            case GripW:  return Qt::LeftEdge;
            case GripNW: return Qt::TopEdge | Qt::LeftEdge;
        }
        return {};
    }

    Qt::CursorShape CursorForGrip(int idx)
    {
        switch (idx)
        {
            case GripN: case GripS:  return Qt::SizeVerCursor;
            case GripE: case GripW:  return Qt::SizeHorCursor;
            case GripNE: case GripSW: return Qt::SizeBDiagCursor;
            case GripSE: case GripNW: return Qt::SizeFDiagCursor;
        }
        return Qt::ArrowCursor;
    }
}

WindowResizeGrips::WindowResizeGrips(QWidget* window) : QObject(window), hostWindow(window)
{
    for (int i = 0; i < 8; i++)
    {
        grips[i] = new QWidget(window);
        grips[i]->setCursor(CursorForGrip(i));
        grips[i]->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        grips[i]->installEventFilter(this);
        grips[i]->raise();
        grips[i]->show();
    }
    window->installEventFilter(this);
    updateGeometry();
}

void WindowResizeGrips::updateGeometry()
{
    const int m = 6; // grip thickness
    int w = hostWindow->width();
    int h = hostWindow->height();

    grips[GripN]->setGeometry(m, 0, w - 2 * m, m);
    grips[GripS]->setGeometry(m, h - m, w - 2 * m, m);
    grips[GripW]->setGeometry(0, m, m, h - 2 * m);
    grips[GripE]->setGeometry(w - m, m, m, h - 2 * m);

    grips[GripNW]->setGeometry(0, 0, m, m);
    grips[GripNE]->setGeometry(w - m, 0, m, m);
    grips[GripSW]->setGeometry(0, h - m, m, m);
    grips[GripSE]->setGeometry(w - m, h - m, m, m);

    bool maximized = hostWindow->isMaximized() || hostWindow->isFullScreen();
    for (int i = 0; i < 8; i++)
        grips[i]->setVisible(!maximized);
}

bool WindowResizeGrips::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == hostWindow && event->type() == QEvent::Resize)
    {
        updateGeometry();
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress)
    {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && hostWindow->windowHandle() && !hostWindow->isMaximized())
        {
            for (int i = 0; i < 8; i++)
            {
                if (obj == grips[i])
                {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                    hostWindow->windowHandle()->startSystemResize(EdgesForGrip(i));
#endif
                    return true;
                }
            }
        }
    }

    return false;
}
