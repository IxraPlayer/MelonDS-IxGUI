#include "LibraryScreen.h"
#include <QFileInfo>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMenu>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QLinearGradient>
#include <cstddef>

#include "NDS_Header.h"

using namespace melonDS;

LibraryScreen::LibraryScreen(QWidget* parent) : QWidget(parent), columns(5), bgHue(0.58)
{
    setObjectName("libraryScreen");

    // Slowly drift the launcher's background hue so it feels alive instead
    // of flat. ~40ms steps with a tiny increment keeps a full cycle around
    // two minutes, which reads as "gently shifting" rather than distracting.
    bgAnimTimer = new QTimer(this);
    connect(bgAnimTimer, &QTimer::timeout, this, [this]()
    {
        bgHue += 0.00035;
        if (bgHue >= 1.0)
            bgHue -= 1.0;
        update();
    });
    bgAnimTimer->start(40);

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

void LibraryScreen::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Two anchor tones, one notch lighter than the old flat #0d0f14
    // background, drifting slowly around the hue wheel together so the
    // gradient direction stays put but the color keeps gently shifting.
    const double hue2 = bgHue + 0.06 > 1.0 ? bgHue + 0.06 - 1.0 : bgHue + 0.06;

    QColor topLeft = QColor::fromHsvF(bgHue, 0.35, 0.115);
    QColor bottomRight = QColor::fromHsvF(hue2, 0.30, 0.145);

    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, topLeft);
    gradient.setColorAt(1.0, bottomRight);

    painter.fillRect(rect(), gradient);

    QWidget::paintEvent(event);
}

QString LibraryScreen::displayName(const QString& path) const
{
    QString name = QFileInfo(path.split('|').first()).completeBaseName();
    return name;
}

QImage LibraryScreen::loadRomIconImage(const QString& path)
{
    // Archive entries ("archive.zip|game.nds") aren't supported for icon
    // extraction yet; fall back to text-only tiles for those.
    if (path.contains('|'))
        return QImage();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QImage();

    NDSHeader header;
    if (file.read(reinterpret_cast<char*>(&header), sizeof(header)) != (qint64)sizeof(header))
        return QImage();

    if (header.BannerOffset == 0)
        return QImage();

    u8 iconData[512];
    u16 palette[16];

    if (!file.seek(header.BannerOffset + offsetof(NDSBanner, Icon)))
        return QImage();
    if (file.read(reinterpret_cast<char*>(iconData), sizeof(iconData)) != (qint64)sizeof(iconData))
        return QImage();

    if (!file.seek(header.BannerOffset + offsetof(NDSBanner, Palette)))
        return QImage();
    if (file.read(reinterpret_cast<char*>(palette), sizeof(palette)) != (qint64)sizeof(palette))
        return QImage();

    u32 paletteRGBA[16];
    for (int i = 0; i < 16; i++)
    {
        u8 r = ((palette[i] >> 0)  & 0x1F) * 255 / 31;
        u8 g = ((palette[i] >> 5)  & 0x1F) * 255 / 31;
        u8 b = ((palette[i] >> 10) & 0x1F) * 255 / 31;
        u8 a = i ? 255 : 0;
        paletteRGBA[i] = r | (g << 8) | (b << 16) | (a << 24);
    }

    u32 iconRGBA[32 * 32];
    int count = 0;
    for (int ytile = 0; ytile < 4; ytile++)
    {
        for (int xtile = 0; xtile < 4; xtile++)
        {
            for (int ypixel = 0; ypixel < 8; ypixel++)
            {
                for (int xpixel = 0; xpixel < 8; xpixel++)
                {
                    u8 pal_index = count % 2 ? iconData[count / 2] >> 4 : iconData[count / 2] & 0x0F;
                    iconRGBA[ytile * 256 + ypixel * 32 + xtile * 8 + xpixel] = paletteRGBA[pal_index];
                    count++;
                }
            }
        }
    }

    QImage img(reinterpret_cast<uchar*>(iconRGBA), 32, 32, QImage::Format_RGBA8888);
    return img.copy();
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
    tile->setContextMenuPolicy(Qt::CustomContextMenu);

    QImage iconImg = loadRomIconImage(path);
    if (!iconImg.isNull())
    {
        QIcon icon(QPixmap::fromImage(iconImg));
        tile->setIcon(icon);
        tile->setIconSize(QSize(64, 64));
    }

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
