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

#include "TopMenuBar.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QPainter>
#include <QLinearGradient>
#include <QRandomGenerator>

// Blue-through-turquoise only, same band used elsewhere in the UI: 0.50 =
// cyan/turquoise, 0.66 = blue. Keeps the glow line from ever drifting into
// red/green/purple.
static constexpr double kGlowHueMin = 0.50;
static constexpr double kGlowHueMax = 0.66;

// growth taken from the hovered button, split evenly and refunded by all
// its siblings so the row's total width stays constant (nothing overflows
// the centered container).
static const int kGrowAmount = 34;
static const int kAnimMs = 140;

TopMenuButton::TopMenuButton(const QString& text, QWidget* parent) : QToolButton(parent)
{
    // A small emoji glyph above the label, picked by matching the button's
    // title, so the bar reads a bit friendlier without needing real icon
    // assets. Falls back to no emoji for anything unrecognized.
    QString emoji;
    const QString lower = text.toLower();
    if (lower.contains("file"))         emoji = QString::fromUtf8("\xF0\x9F\x93\x81");             // 📁
    else if (lower.contains("system"))  emoji = QString::fromUtf8("\xF0\x9F\x92\xBB");             // 💻
    else if (lower.contains("view"))    emoji = QString::fromUtf8("\xF0\x9F\x96\xA5\xEF\xB8\x8F");  // 🖥️
    else if (lower.contains("config"))  emoji = QString::fromUtf8("\xE2\x9A\x99\xEF\xB8\x8F");      // ⚙️
    else if (lower.contains("help"))    emoji = QString::fromUtf8("\xE2\x9D\x93");                  // ❓

    setText(emoji.isEmpty() ? text : emoji + "\n" + text);
    setObjectName("topMenuButton");
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setPopupMode(QToolButton::InstantPopup);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);
    setFixedWidth(m_baseWidth);
    setFixedHeight(48);
}

void TopMenuButton::enterEvent(MenuBtnEnterEvent* event)
{
    emit hoverChanged(this, true);
    QToolButton::enterEvent(event);
}

void TopMenuButton::leaveEvent(QEvent* event)
{
    emit hoverChanged(this, false);
    QToolButton::leaveEvent(event);
}

void TopMenuButton::mousePressEvent(QMouseEvent* event)
{
    // tiny "sinks in" nudge, purely visual, restored on release
    move(pos().x(), pos().y() + 1);
    QToolButton::mousePressEvent(event);
}

void TopMenuButton::mouseReleaseEvent(QMouseEvent* event)
{
    move(pos().x(), pos().y() - 1);
    QToolButton::mouseReleaseEvent(event);
}

void TopMenuButton::animateWidthTo(int target)
{
    // We own this animation outright (no DeleteWhenStopped): stop+delete the
    // old one synchronously before making a new one, so there is never a
    // window where a stale/auto-deleted pointer could be touched again.
    if (m_anim)
    {
        m_anim->stop();
        delete m_anim;
        m_anim = nullptr;
    }

    m_anim = new QPropertyAnimation(this, "barWidth", this);
    m_anim->setDuration(kAnimMs);
    m_anim->setStartValue(width());
    m_anim->setEndValue(target);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
    m_anim->start();
}

TopMenuButton::~TopMenuButton()
{
    delete m_anim;
    m_anim = nullptr;
}

TopMenuBar::TopMenuBar(QWidget* parent) : QWidget(parent),
    glowHue(0.58), glowTargetHue(0.58), glowRetargetTicks(0)
{
    setObjectName("topMenuBar");
    setFixedHeight(58);

    auto* layout = new QHBoxLayout(this);
    // Extra top margin nudges the row down a bit within the taller bar
    // instead of sitting flush against the title bar above it. Extra
    // bottom margin leaves room for the glow line under the buttons.
    layout->setContentsMargins(0, 8, 0, 6);
    layout->setSpacing(6);
    layout->addStretch(1);
    // buttons get inserted before this trailing stretch by addMenuButton()
    layout->addStretch(1);

    glowTimer = new QTimer(this);
    connect(glowTimer, &QTimer::timeout, this, [this]()
    {
        // Every ~1.8s pick a new random target hue within the band, easing
        // toward it each tick so the drift reads as smooth rather than a
        // hard jump.
        glowRetargetTicks++;
        if (glowRetargetTicks >= 45)
        {
            glowRetargetTicks = 0;
            double span = kGlowHueMax - kGlowHueMin;
            glowTargetHue = kGlowHueMin + QRandomGenerator::global()->generateDouble() * span;
        }
        glowHue += (glowTargetHue - glowHue) * 0.03;

        update();
    });
    glowTimer->start(40);
}

void TopMenuBar::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor lineColor = QColor::fromHsvF(glowHue, 0.70, 0.95);

    const qreal y = height() - 2.0;

    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0.0, QColor(lineColor.red(), lineColor.green(), lineColor.blue(), 0));
    gradient.setColorAt(0.5, lineColor);
    gradient.setColorAt(1.0, QColor(lineColor.red(), lineColor.green(), lineColor.blue(), 0));

    // A few soft passes with falling alpha/width fake a glow without
    // needing a graphics effect.
    struct GlowPass { qreal width; qreal alphaScale; };
    const GlowPass passes[] = { {6.0, 0.25}, {3.0, 0.55}, {1.4, 1.0} };
    for (const auto& pass : passes)
    {
        QPen pen(QBrush(gradient), pass.width);
        pen.setCapStyle(Qt::FlatCap);
        painter.setOpacity(pass.alphaScale);
        painter.setPen(pen);
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    }
}

TopMenuButton* TopMenuBar::addMenuButton(const QString& text, QMenu* menu)
{
    auto* btn = new TopMenuButton(text, this);
    if (menu)
        btn->setMenu(menu);

    auto* layout = static_cast<QHBoxLayout*>(this->layout());
    // insert right before the trailing stretch (last item)
    layout->insertWidget(layout->count() - 1, btn);

    buttons.append(btn);
    connect(btn, &TopMenuButton::hoverChanged, this, &TopMenuBar::onHoverChanged);

    return btn;
}

void TopMenuBar::onHoverChanged(TopMenuButton* hovered, bool isHover)
{
    int shrinkEach = buttons.size() > 1 ? kGrowAmount / (buttons.size() - 1) : 0;

    for (auto* btn : buttons)
    {
        int target = btn->baseWidth();
        if (isHover)
        {
            if (btn == hovered)
                target = btn->baseWidth() + kGrowAmount;
            else
                target = btn->baseWidth() - shrinkEach;
        }

        btn->animateWidthTo(target);
    }
}
