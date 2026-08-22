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

#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;

// Shown once, the very first time the launcher runs (before the library
// screen is shown), so a new user picks a nickname and a UI language right
// away instead of stumbling onto those settings buried in Config later.
// MainWindow gates this behind the "OnboardingDone" flag in the global
// config so it never appears again after the first run.
class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget* parent);

    QString chosenName() const;
    // Language code ("" = system default, "en", "tr", ...), same values
    // used by InterfaceSettingsDialog's cbxUILanguage.
    QString chosenLanguageCode() const;

private:
    QLineEdit* nameEdit;
    QComboBox* languageBox;
};

#endif // WELCOMEDIALOG_H
