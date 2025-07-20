/*!
 * @file plugin-dock.cpp
 * @brief Implements the PlayFameDock widget for OBS, containing UI setup and dock registration.
 *
 * @author <Developer> <Email Address>
 * @copyright Copyright (C) <Year> <Developer>
 * @license GNU General Public License v2 or later
 * @see https://www.gnu.org/licenses/
 */

#include "plugin-dock.h"
#include <QVBoxLayout>
#include <QLabel>
#include "config-dialog.h"


/**
 * @brief Constructs the PlayFameDock.
 *
 * Sets up the dock's UI, including layout and widgets, parented to the given OBS main window.
 *
 * @param parent The OBS main window widget to attach this dock to.
 */
PlayFameDock::PlayFameDock(OBSConfigHelper *cfg, QWidget *parent)
    : QWidget(parent)
    , cfg_(cfg)
{
    setWindowTitle(kDockName);
    auto *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto *cfgBtn = new QPushButton("Config", this);
    cfgBtn->setMinimumWidth(120);
    cfgBtn->setStyleSheet(
        "QPushButton {"
        "  padding:6px 14px; border-radius:6px;"
        "  background:#2d89ef; color:white;"
        "  font-weight:600;"
        "}"
        "QPushButton:hover { background:#368af0; }");
    layout->addWidget(cfgBtn);

    connect(cfgBtn, &QPushButton::clicked, this, [this]() {
        ConfigDialog dlg(cfg_, this);
        dlg.exec();
    });

    setLayout(layout);
}

/**
 * @brief Destructor for PlayFameDock.
 *
 * Does not unregister the dock; cleanup is handled in unregisterDock which is called by obs_module_unload() in plugin-main.cpp.
 */
PlayFameDock::~PlayFameDock()
{
    // Do NOT unregister here; obs_module_unload() handles that.
}

/**
 * @brief Registers this dock with the OBS frontend.
 *
 * Calls OBS API to add this widget as a dockable panel.
 *
 * @return true if registration succeeded, false otherwise.
 */
bool PlayFameDock::registerDock()
{
    return obs_frontend_add_dock_by_id(kDockId, kDockName, this);
}

/**
 * @brief Unregisters this dock from the OBS frontend.
 *
 * Calls OBS API to remove this dockable panel.  Is called by obs_module_unload() in plugin-main.cpp.
 */
void PlayFameDock::unregisterDock()
{
    obs_frontend_remove_dock(kDockId);
}
