#ifndef LIBRARYSCREEN_H
#define LIBRARYSCREEN_H

#include <QWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QStringList>
#include <QMap>

class LibraryScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryScreen(QWidget* parent);

    void addGame(const QString& path);
    QStringList gamePaths() const { return paths; }

signals:
    void romActivated(QString path);
    void addGameRequested();
    void libraryChanged();

private:
    void relayout();
    QString displayName(const QString& path) const;

    QGridLayout* grid;
    QToolButton* addTile;
    QStringList paths;
    QMap<QString, QToolButton*> tiles;
    int columns;
};

#endif
