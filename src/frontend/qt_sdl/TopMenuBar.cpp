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

// growth taken from the hovered button, split evenly and refunded by all
// its siblings so the row's total width stays constant (nothing overflows
// the centered container).
static const int kGrowAmount = 34;
static const int kAnimMs = 140;

TopMenuButton::TopMenuButton(const QString& text, QWidget* parent) : QToolButton(parent)
{
    setText(text);
    setObjectName("topMenuButton");
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setPopupMode(QToolButton::InstantPopup);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);
    setFixedWidth(m_baseWidth);
    setFixedHeight(34);
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

TopMenuBar::TopMenuBar(QWidget* parent) : QWidget(parent)
{
    setObjectName("topMenuBar");
    setFixedHeight(40);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 4);
    layout->setSpacing(6);
    layout->addStretch(1);
    // buttons get inserted before this trailing stretch by addMenuButton()
    layout->addStretch(1);
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
    for (auto* anim : anims)
        anim->stop();
    anims.clear();

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

        auto* anim = new QPropertyAnimation(btn, "barWidth", this);
        anim->setDuration(kAnimMs);
        anim->setStartValue(btn->width());
        anim->setEndValue(target);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anims.append(anim);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
