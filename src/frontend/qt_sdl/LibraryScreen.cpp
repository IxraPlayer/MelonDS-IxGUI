#include "LibraryScreen.h"
#include <QFileInfo>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMenu>

LibraryScreen::LibraryScreen(QWidget* parent) : QWidget(parent), columns(5)
{
    setObjectName("libraryScreen");

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* inner = new QWidget();
    grid = new QGridLayout(inner);
    grid->setSpacing(18);
    grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scroll->setWidget(inner);
    outer->addWidget(scroll);

    addTile = new QToolButton(this);
    addTile->setObjectName("addGameTile");
    addTile->setText("+");
    addTile->setFixedSize(140, 140);
    addTile->setToolButtonStyle(Qt::ToolButtonTextOnly);
    connect(addTile, &QToolButton::clicked, this, &LibraryScreen::addGameRequested);

    grid->addWidget(addTile, 0, 0);
}

QString LibraryScreen::displayName(const QString& path) const
{
    QString name = QFileInfo(path.split('|').first()).completeBaseName();
    return name;
}

void LibraryScreen::addGame(const QString& path)
{
    if (paths.contains(path))
        return;

    paths.append(path);

    auto* tile = new QToolButton(this);
    tile->setObjectName("gameCard");
    tile->setText(displayName(path));
    tile->setFixedSize(140, 140);
    tile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tile->setWordWrap(true);
    tile->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tile, &QToolButton::clicked, this, [this, path]()
    {
        emit romActivated(path);
    });

    connect(tile, &QToolButton::customContextMenuRequested, this, [this, tile, path](const QPoint& pos)
    {
        QMenu menu(tile);
        QAction* removeAct = menu.addAction("Remove from library");
        QAction* chosen = menu.exec(tile->mapToGlobal(pos));
        if (chosen == removeAct)
        {
            paths.removeAll(path);
            tiles.remove(path);
            tile->deleteLater();
            relayout();
            emit libraryChanged();
        }
    });

    tiles.insert(path, tile);
    relayout();
}

void LibraryScreen::relayout()
{
    grid->removeWidget(addTile);

    int index = 0;
    for (const QString& path : paths)
    {
        QToolButton* tile = tiles.value(path, nullptr);
        if (!tile) continue;

        grid->removeWidget(tile);
        grid->addWidget(tile, index / columns, index % columns);
        index++;
    }

    grid->addWidget(addTile, index / columns, index % columns);
}
