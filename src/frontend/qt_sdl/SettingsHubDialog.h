#ifndef SETTINGSHUBDIALOG_H
#define SETTINGSHUBDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>

class SettingsHubDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsHubDialog(QWidget* parent);
    int addCategory(const QString& title);

    // Embeds the given widget into the right-hand panel, replacing whatever
    // page is currently shown there. The hub takes ownership of the widget.
    void setPage(QWidget* page);

signals:
    void categorySelected(int index);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* sidebar;
    QStackedWidget* stack;
    QWidget* placeholder;
};

#endif
