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

    // The page must never have been shown as a top-level window before this
    // point - stripping window flags off a dialog that's already running as
    // a modal window is what caused the freeze. Callers must hand us a
    // freshly-constructed dialog that hasn't had open()/show()/exec() called.
    page->setWindowFlags(Qt::Widget);
    stack->addWidget(page);
    stack->setCurrentWidget(page);

    // The embedded page was originally designed as a standalone dialog, so
    // it carries its own preferred size. If the hub window is smaller than
    // that, the page gets squeezed into the right-hand panel and everything
    // inside it looks cramped until the user manually enlarges the window.
    // Grow (never shrink below the base minimum) to comfortably fit whatever
    // page is currently shown.
    QSize pageHint = page->sizeHint().expandedTo(page->minimumSizeHint());

    const int sidebarWidth = sidebar->width();
    const int rightMarginsW = 24 + 24;   // left/right content margins
    const int rightMarginsH = 20 + 16;   // top/bottom content margins
    const int closeRowH = 40;            // close button row + spacing

    int neededW = sidebarWidth + rightMarginsW + pageHint.width() + 8;
    int neededH = rightMarginsH + pageHint.height() + closeRowH;

    QSize base(680, 480);
    QSize target = base.expandedTo(QSize(neededW, neededH));

    if (target.width() > width() || target.height() > height())
        resize(target.expandedTo(size()));
}
