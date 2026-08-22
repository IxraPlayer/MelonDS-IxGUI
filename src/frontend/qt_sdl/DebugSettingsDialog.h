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

#ifndef DEBUGSETTINGSDIALOG_H
#define DEBUGSETTINGSDIALOG_H

#include <QDialog>

class MainWindow;
class EmuInstance;

// Settings > Debug settings. Intentionally minimal: the only thing here is
// the keyboard hotkey used to toggle the in-game FPS/CPU/RAM overlay
// (HK_ToggleDebugOverlay). Everything else about the overlay itself lives
// in ScreenPanel (Screen.h/.cpp) - this dialog just lets the user bind a
// key to it, same as any other hotkey in Input and hotkeys.
class DebugSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DebugSettingsDialog(QWidget* parent);
    ~DebugSettingsDialog();

private slots:
    void done(int r) override;

private:
    MainWindow* mainWindow;
    EmuInstance* emuInstance;

    int hkKeyMapping;
};

#endif // DEBUGSETTINGSDIALOG_H
