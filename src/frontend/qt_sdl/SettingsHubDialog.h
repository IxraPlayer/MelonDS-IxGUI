#ifndef SETTINGSHUBDIALOG_H
#define SETTINGSHUBDIALOG_H

#include <QDialog>
#include <QListWidget>

class SettingsHubDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsHubDialog(QWidget* parent);
    int addCategory(const QString& title);

signals:
    void categorySelected(int index);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* sidebar;
};

#endif
