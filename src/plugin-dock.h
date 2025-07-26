/*!
 * @file plugin-dock.h
 * @brief Declaration of the PlayFameDock widget for OBS frontend integration.
 *
 * @author <Developer> <Email Address>
 * @copyright Copyright (C) <Year> <Developer>
 * @license GNU General Public License v2 or later
 * @see https://www.gnu.org/licenses/
 */

#pragma once

#include <obs-frontend-api.h>
#include "obs-config-helper.h"
#include <QWidget>
#include <QVBoxLayout>
#include <memory>
#include <vector>

class StreamOutput;

/**
 * @class PlayFameDock
 * @brief Dockable widget for the PlayFame plugin within OBS.
 *
 * Inherits from QWidget and provides UI integration with the OBS main window.
 */
class PlayFameDock : public QWidget {
    Q_OBJECT

public:
    explicit PlayFameDock(OBSConfigHelper *cfg, QWidget *parent = nullptr);
    ~PlayFameDock() override;

    /// Register the dock with OBS. Returns true on success.
    bool registerDock();

    /// Unregister the dock from OBS.
    void unregisterDock();

private:
    static constexpr const char *kDockId   = "playfame_dock";
    static constexpr const char *kDockName = "PlayFame";
    OBSConfigHelper *cfg_;   

    QVBoxLayout *outputsLayout_{nullptr};
    std::vector<std::unique_ptr<StreamOutput>> outputs_;

    void setupUI();
    void createOutputs();
};

