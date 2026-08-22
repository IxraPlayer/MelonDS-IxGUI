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

#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QColor>
#include <QPoint>

// Small vector "melon slice" logo painted with the app's blue/turquoise
// palette. Deliberately not an emoji/glyph -- pure vector paint, so it
// never depends on font/emoji rendering being available.
class TitleBarLogo : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBarLogo(QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
};

// A titlebar button (minimize/maximize/close) that visually "presses down"
// on click and glows softly on hover. Kept generic so the close button can
// override the hover color via setHoverColor().
class TitleBarButton : public QToolButton
{
    Q_OBJECT
public:
    explicit TitleBarButton(const QString& glyphPath, QWidget* parent = nullptr);
    void setHoverColor(const QColor& c) { hoverColor = c; update(); }
    void setGlyph(const QString& glyphPath) { glyph = glyphPath; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QString glyph; // "min" | "max" | "restore" | "close"
    QColor hoverColor { 61, 90, 254 };
    bool pressedDown = false;
};

// Custom draggable titlebar replacing the OS one: app logo + window title +
// minimize/maximize/close. Uses QWindow::startSystemMove()/startSystemResize()
// so dragging and edge-resizing are handled natively by the compositor/WM.
class CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTitleBar(QWidget* window, QWidget* parent = nullptr);

    void setTitleText(const QString& title);
    void refreshMaximizeGlyph();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* hostWindow;
    QLabel* titleLabel;
    TitleBarButton* btnMin;
    TitleBarButton* btnMax;
    TitleBarButton* btnClose;
    QPoint dragOrigin;
    bool dragging = false;
};

// Installs invisible edge/corner grip strips around a top-level window so a
// frameless window can still be resized by dragging its borders, same as a
// normal OS-decorated window would allow.
class WindowResizeGrips : public QObject
{
    Q_OBJECT
public:
    explicit WindowResizeGrips(QWidget* window);
    void updateGeometry();

private:
    QWidget* hostWindow;
    QWidget* grips[8]; // N, NE, E, SE, S, SW, W, NW
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // CUSTOMTITLEBAR_H
