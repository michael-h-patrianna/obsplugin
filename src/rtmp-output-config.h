/*!
 * @file rtmp-output-config.h
 * @brief Declaration of RTMP-specific output configuration.
 */

#pragma once

#include "output-config.h"

/**
 * @brief Configuration specific to RTMP outputs (YouTube, Twitch, Facebook, Kick)
 */
class RTMPOutputConfig : public OutputConfig {
public:
    explicit RTMPOutputConfig();
    ~RTMPOutputConfig() override;

    // RTMP-specific fields
    QString serverUrl;
    QString backupServerUrl;
    QString streamKey;
    bool useAutoReconnect{true};

    void loadFromObs(obs_data_t *data) override;
    void saveToObs(obs_data_t *data) const override;
    bool validate() const override;

private:
    static bool validateRtmpUrl(const QString &url);
}; 

