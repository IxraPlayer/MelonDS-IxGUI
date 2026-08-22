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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

#include "types.h"
#include "Config.h"
#include "main.h"

#include "DebugSettingsDialog.h"
#include "EmuInstance.h"
#include "InputConfig/MapButton.h"

DebugSettingsDialog::DebugSettingsDialog(QWidget* parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Debug settings");

    mainWindow = (MainWindow*)parent;
    emuInstance = mainWindow->getEmuInstance();

    Config::Table& instcfg = emuInstance->getLocalConfig();
    Config::Table keycfg = instcfg.GetTable("Keyboard");
    hkKeyMapping = keycfg.GetInt(EmuInstance::hotkeyNames[HK_ToggleDebugOverlay]);

    auto* layout = new QVBoxLayout(this);

    auto* group = new QGroupBox(tr("Debug overlay"));
    auto* groupLayout = new QVBoxLayout(group);

    auto* hint = new QLabel(tr(
        "Assign a key to show/hide the in-game FPS, CPU and RAM overlay.\n"
        "Everything else on this page is intentionally left empty."));
    hint->setWordWrap(true);
    groupLayout->addWidget(hint);

    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("Toggle debug overlay")));
    row->addStretch();
    auto* keyBtn = new KeyMapButton(&hkKeyMapping, true);
    row->addWidget(keyBtn);
    groupLayout->addLayout(row);

    layout->addWidget(group);
    layout->addStretch();
}

DebugSettingsDialog::~DebugSettingsDialog()
{
}

void DebugSettingsDialog::done(int r)
{
    // Same behavior regardless of Ok/Cancel/close-button: this is a live
    // hotkey binding, not a form with a "discard changes" concept, so we
    // always persist it - consistent with how the rest of the hotkeys
    // (Input and hotkeys tab) work.
    Config::Table& instcfg = emuInstance->getLocalConfig();
    Config::Table keycfg = instcfg.GetTable("Keyboard");
    keycfg.SetInt(EmuInstance::hotkeyNames[HK_ToggleDebugOverlay], hkKeyMapping);

    emuInstance->inputLoadConfig();

    QDialog::done(r);
}
