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
#include "plugin-support.h"
#include "stream-output.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFrame>
#include <obs-frontend-api.h>
#include <obs-module.h>

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
    setupUI();
    createOutputs();
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

void PlayFameDock::setupUI()
{
    // Apply stylesheet with correct Qt widget selectors

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    auto *scrollWidget = new QWidget(scrollArea);
    scrollWidget->setContentsMargins(10, 10, 10, 10); // Some padding around dock
    scrollArea->setWidget(scrollWidget);
    scrollWidget->setStyleSheet("background-color: #13141A;");
    
    outputsLayout_ = new QVBoxLayout(scrollWidget);
    outputsLayout_->setContentsMargins(0, 0, 0, 0);
    outputsLayout_->setSpacing(10); // gap: 10px between entries as in HTML
    outputsLayout_->setAlignment(Qt::AlignTop);

    mainLayout->addWidget(scrollArea);
}

void PlayFameDock::createOutputs()
{
    obs_log(LOG_INFO, "[PlayFameDock] Creating outputs...");
    
    // Create outputs in order specified by the design: PlayFame, YouTube, Twitch, Facebook, Kick
    try {
        outputs_.emplace_back(std::make_unique<StreamOutput>(cfg_, "PlayFame", "playfame_icon.png", this));
        obs_log(LOG_INFO, "[PlayFameDock] Created PlayFame output");
        
        outputs_.emplace_back(std::make_unique<StreamOutput>(cfg_, "YouTube", "youtube_icon.png", this));
        obs_log(LOG_INFO, "[PlayFameDock] Created YouTube output");
        
        outputs_.emplace_back(std::make_unique<StreamOutput>(cfg_, "Twitch", "twitch_icon.png", this));
        obs_log(LOG_INFO, "[PlayFameDock] Created Twitch output");
        
        outputs_.emplace_back(std::make_unique<StreamOutput>(cfg_, "Facebook", "facebook_icon.png", this));
        obs_log(LOG_INFO, "[PlayFameDock] Created Facebook output");
        
        outputs_.emplace_back(std::make_unique<StreamOutput>(cfg_, "Kick", "kick_icon.png", this));
        obs_log(LOG_INFO, "[PlayFameDock] Created Kick output");
    } catch (const std::exception& e) {
        obs_log(LOG_WARNING, "[PlayFameDock] Error creating outputs: %s", e.what());
        return;
    }

    obs_log(LOG_INFO, "[PlayFameDock] Created %zu outputs", outputs_.size());

    // Initialize all outputs
    for (size_t i = 0; i < outputs_.size(); ++i) {
        try {
            outputs_[i]->initialize();
            obs_log(LOG_INFO, "[PlayFameDock] Initialized output %zu: %s", i, outputs_[i]->serviceName().toUtf8().constData());
        } catch (const std::exception& e) {
            obs_log(LOG_WARNING, "[PlayFameDock] Error initializing output %zu: %s", i, e.what());
        }
    }

    // Set demo states to match the design: offline, unused, connecting, error, online  
    if (outputs_.size() >= 5) {
        obs_log(LOG_INFO, "[PlayFameDock] Setting demo states...");
        outputs_[0]->setState(OutputState::Offline);    // PlayFame - offline
        outputs_[1]->setState(OutputState::Unused);     // YouTube - unused  
        outputs_[2]->setState(OutputState::Connecting); // Twitch - connecting
        outputs_[3]->setState(OutputState::Error);      // Facebook - error
        outputs_[4]->setState(OutputState::Online);     // Kick - online
        obs_log(LOG_INFO, "[PlayFameDock] Demo states set");
    }

    // Add outputs to layout
    for (size_t i = 0; i < outputs_.size(); ++i) {
        try {
            outputsLayout_->addWidget(outputs_[i].get());
            obs_log(LOG_INFO, "[PlayFameDock] Added output %zu to layout: %s", i, outputs_[i]->serviceName().toUtf8().constData());
        } catch (const std::exception& e) {
            obs_log(LOG_WARNING, "[PlayFameDock] Error adding output %zu to layout: %s", i, e.what());
        }
    }
    
    obs_log(LOG_INFO, "[PlayFameDock] All outputs created and added to layout");
}

