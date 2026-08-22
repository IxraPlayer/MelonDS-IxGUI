#ifndef LIBRARYSCREEN_H
#define LIBRARYSCREEN_H

#include <QWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QStringList>
#include <QMap>
#include <QIcon>
#include <QImage>
#include <QTimer>
#include <QColor>

class LibraryScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryScreen(QWidget* parent);

    void addGame(const QString& path);
    QStringList gamePaths() const { return paths; }

    // Decodes a plain .nds ROM's banner icon (32x32, NDS-native palette) into
    // a QImage. Shared with Window.cpp so desktop shortcuts can use the same
    // icon as the library tile. Returns a null QImage on failure (missing
    // banner, archive entries, unreadable file, etc).
    static QImage loadRomIconImage(const QString& path);

signals:
    void romActivated(QString path);
    void addGameRequested();
    void libraryChanged();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void relayout();
    QString displayName(const QString& path) const;

    QGridLayout* grid;
    QToolButton* addTile;
    QStringList paths;
    QMap<QString, QToolButton*> tiles;
    int columns;

    double bgHue;
};

#endif
