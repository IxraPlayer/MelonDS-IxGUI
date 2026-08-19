#include "SettingsHubDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

SettingsHubDialog::SettingsHubDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Settings");
    setMinimumSize(680, 480);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    sidebar = new QListWidget(this);
    sidebar->setObjectName("sidebarPanel");
    sidebar->setFrameShape(QFrame::NoFrame);
    sidebar->setFixedWidth(220);
    sidebar->setSpacing(2);
    root->addWidget(sidebar);

    auto* right = new QVBoxLayout();
    right->setContentsMargins(24, 20, 24, 16);

    stack = new QStackedWidget(this);

    placeholder = new QWidget(stack);
    auto* placeholderLayout = new QVBoxLayout(placeholder);
    auto* title = new QLabel("Select a category on the left");
    title->setObjectName("libraryTitle");
    placeholderLayout->addWidget(title);
    placeholderLayout->addStretch();
    stack->addWidget(placeholder);

    right->addWidget(stack, 1);

    auto* closeRow = new QHBoxLayout();
    closeRow->addStretch();
    auto* closeBtn = new QPushButton("Close");
    closeBtn->setObjectName("primaryButton");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    closeRow->addWidget(closeBtn);
    right->addLayout(closeRow);

    root->addLayout(right, 1);

    connect(sidebar, &QListWidget::itemClicked, this, &SettingsHubDialog::onItemClicked);
}

int SettingsHubDialog::addCategory(const QString& title)
{
    auto* item = new QListWidgetItem(title, sidebar);
    item->setSizeHint(QSize(0, 40));
    return sidebar->count() - 1;
}

void SettingsHubDialog::onItemClicked(QListWidgetItem* item)
{
    emit categorySelected(sidebar->row(item));
}

void SettingsHubDialog::setPage(QWidget* page)
{
    // Drop whatever page (other than the placeholder) is currently embedded.
    for (int i = stack->count() - 1; i >= 0; i--)
    {
        QWidget* w = stack->widget(i);
        if (w != placeholder && w != page)
        {
            stack->removeWidget(w);
            w->deleteLater();
        }
    }

    if (stack->indexOf(page) == -1)
    {
        page->setParent(stack);
        page->setWindowFlags(Qt::Widget);
        stack->addWidget(page);
    }

    stack->setCurrentWidget(page);
    page->show();
}
