/*!
 * @file output-config.h
 * @brief Declaration of the base output configuration class.
 */

#pragma once

#include <QString>
#include <QVariant>
#include <obs.hpp>

/**
 * @brief Base class for output configuration, containing common fields for all outputs
 */
class OutputConfig {
public:
    explicit OutputConfig();
    virtual ~OutputConfig();

    // Common configuration fields as per spec section 5.1
    bool enabled{true};
    QString customTitle;
    QString sceneId;
    QString videoEncoderId;
    int videoBitrateKbps{2500};
    int keyframeIntervalSec{2};
    QString encoderProfile;
    QString encoderPreset;
    QString resolution{"1280x720"};
    int fps{60};
    QString audioEncoderId;
    int audioBitrateKbps{128};
    QString audioChannels{"stereo"};
    int audioSampleRate{48000};
    int reconnectDelayMs{3000};
    int maxRetries{20};
    QString customParamsJson;

    /// Load configuration from OBS settings
    virtual void loadFromObs(obs_data_t *data);

    /// Save configuration to OBS settings
    virtual void saveToObs(obs_data_t *data) const;

    /// Validate the configuration
    virtual bool validate() const;

    /// Get a list of available video encoders
    static QStringList availableVideoEncoders();

    /// Get a list of available audio encoders
    static QStringList availableAudioEncoders();

    /// Get a list of available scenes
    static QStringList availableScenes();

    /// Get supported profiles for a given encoder
    static QStringList supportedProfiles(const QString &encoderId);

    /// Get supported presets for a given encoder
    static QStringList supportedPresets(const QString &encoderId);

protected:
    /// Helper to validate resolution string format
    static bool validateResolution(const QString &res);

    /// Helper to validate JSON string
    static bool validateJson(const QString &json);
}; 

