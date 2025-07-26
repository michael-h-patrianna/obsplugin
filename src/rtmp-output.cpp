/*!
 * @file rtmp-output.cpp
 * @brief Implementation of the base RTMP output class.
 */

#include "rtmp-output.h"
#include "plugin-support.h"
#include "rtmp-output-config.h"
#include "config-dialog.h"
#include "obs-config-helper.h"

#include <obs.h>
#include <obs-module.h>

RTMPOutput::RTMPOutput(OBSConfigHelper *cfg, const QString &serviceName, const QString &serviceIconPath, QWidget *parent)
    : StreamOutput(cfg, serviceName, serviceIconPath, parent)
    , config_(std::make_unique<RTMPOutputConfig>())
{
    // Constructor only initializes member variables
    // UI setup will be done in initialize() method
}

RTMPOutput::~RTMPOutput()
{
    releaseOutput();
}

void RTMPOutput::initialize()
{
    // Call base class initialization first
    StreamOutput::initialize();
    
    // Now we can safely load config since derived class is fully constructed
    loadConfig();
}

void RTMPOutput::showConfig()
{
    ConfigDialog dlg(cfg_, this);
    if (dlg.exec() == QDialog::Accepted) {
        saveConfig();
        setState(validateConfig() ? OutputState::Offline : OutputState::Unused);
    }
}

void RTMPOutput::startStream()
{
    if (!validateConfig()) {
        setState(OutputState::Error);
        return;
    }

    setupRtmpOutput();
    if (!output_) {
        setState(OutputState::Error);
        return;
    }

    setState(OutputState::Connecting);
    obs_output_start(output_);
}

void RTMPOutput::stopStream()
{
    if (output_ && obs_output_active(output_)) {
        obs_output_stop(output_);
    }
    setState(OutputState::Offline);
}

void RTMPOutput::loadConfig()
{
    if (!cfg_) {
        obs_log(LOG_WARNING, "[RTMPOutput] No config helper available");
        setState(OutputState::Unused);
        return;
    }

    // Load configuration from global config using service-specific keys
    QString prefix = serviceName() + "_";
    
    config_->enabled = cfg_->getValue("outputs", prefix + "enabled", true).toBool();
    config_->customTitle = cfg_->getValue("outputs", prefix + "title", "").toString();
    config_->videoBitrateKbps = cfg_->getValue("outputs", prefix + "video_bitrate", 2500).toInt();
    config_->audioBitrateKbps = cfg_->getValue("outputs", prefix + "audio_bitrate", 128).toInt();
    config_->resolution = cfg_->getValue("outputs", prefix + "resolution", "1280x720").toString();
    config_->fps = cfg_->getValue("outputs", prefix + "fps", 60).toInt();
    
    // RTMP-specific config
    config_->serverUrl = cfg_->getValue("outputs", prefix + "server_url", defaultServerUrl()).toString();
    config_->streamKey = cfg_->getValue("outputs", prefix + "stream_key", "").toString();
    config_->useAutoReconnect = cfg_->getValue("outputs", prefix + "auto_reconnect", true).toBool();

    // Set initial state based on config validity
    setState(validateConfig() ? OutputState::Offline : OutputState::Unused);
}

void RTMPOutput::saveConfig()
{
    if (!cfg_) {
        obs_log(LOG_WARNING, "[RTMPOutput] No config helper available");
        return;
    }

    // Save configuration to global config using service-specific keys
    QString prefix = serviceName() + "_";
    
    // Use the simple setValue overload (void return type) by calling it directly
    auto setValue = static_cast<void(OBSConfigHelper::*)(const QString&, const QString&, const QVariant&)>
        (&OBSConfigHelper::setValue);
    
    (cfg_->*setValue)("outputs", prefix + "enabled", config_->enabled);
    (cfg_->*setValue)("outputs", prefix + "title", config_->customTitle);
    (cfg_->*setValue)("outputs", prefix + "video_bitrate", config_->videoBitrateKbps);
    (cfg_->*setValue)("outputs", prefix + "audio_bitrate", config_->audioBitrateKbps);
    (cfg_->*setValue)("outputs", prefix + "resolution", config_->resolution);
    (cfg_->*setValue)("outputs", prefix + "fps", config_->fps);
    
    // RTMP-specific config
    (cfg_->*setValue)("outputs", prefix + "server_url", config_->serverUrl);
    (cfg_->*setValue)("outputs", prefix + "stream_key", config_->streamKey);
    (cfg_->*setValue)("outputs", prefix + "auto_reconnect", config_->useAutoReconnect);
}

bool RTMPOutput::validateConfig() const
{
    return config_ && config_->validate();
}

void RTMPOutput::setupRtmpOutput()
{
    releaseOutput();

    output_ = obs_output_create("rtmp_output", "rtmp_output", nullptr, nullptr);
    if (!output_) return;

    // Set up output handlers
    signal_handler_t *handler = obs_output_get_signal_handler(output_);
    signal_handler_connect(handler, "start", obsOutputStarted, this);
    signal_handler_connect(handler, "stop", obsOutputStopped, this);
    signal_handler_connect(handler, "reconnect", obsOutputReconnect, this);
    signal_handler_connect(handler, "reconnect_success", obsOutputReconnectSuccess, this);

    // Configure output settings
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "server", config_->serverUrl.toUtf8().constData());
    obs_data_set_string(settings, "key", config_->streamKey.toUtf8().constData());

    if (!config_->backupServerUrl.isEmpty()) {
        obs_data_set_string(settings, "backup_server", config_->backupServerUrl.toUtf8().constData());
    }

    if (config_->useAutoReconnect) {
        obs_data_set_bool(settings, "auto_reconnect", true);
        obs_data_set_int(settings, "reconnect_delay_sec", config_->reconnectDelayMs / 1000);
        obs_data_set_int(settings, "max_retries", config_->maxRetries);
    }

    obs_output_update(output_, settings);
    obs_data_release(settings);

    // Set up encoders
    obs_encoder_t *vencoder = obs_video_encoder_create(
        config_->videoEncoderId.toUtf8().constData(), "video-encoder", nullptr, nullptr);
    setupAudioEncoder();

    if (vencoder) {
        // Configure video encoder
        obs_data_t *vsettings = obs_data_create();
        obs_data_set_int(vsettings, "bitrate", config_->videoBitrateKbps);
        obs_data_set_int(vsettings, "keyint_sec", config_->keyframeIntervalSec);
        if (!config_->encoderProfile.isEmpty()) {
            obs_data_set_string(vsettings, "profile", config_->encoderProfile.toUtf8().constData());
        }
        if (!config_->encoderPreset.isEmpty()) {
            obs_data_set_string(vsettings, "preset", config_->encoderPreset.toUtf8().constData());
        }
        obs_encoder_update(vencoder, vsettings);
        obs_data_release(vsettings);

        // Attach encoders to output
        obs_output_set_video_encoder(output_, vencoder);
    }

    obs_encoder_release(vencoder);
}

void RTMPOutput::releaseOutput()
{
    if (output_) {
        obs_output_release(output_);
        output_ = nullptr;
    }
}

void RTMPOutput::obsOutputStarted(void *data, calldata_t *)
{
    auto *output = static_cast<RTMPOutput*>(data);
    output->setState(OutputState::Online);
}

void RTMPOutput::obsOutputStopped(void *data, calldata_t *params)
{
    auto *output = static_cast<RTMPOutput*>(data);
    int code = calldata_int(params, "code");
    if (code != 0) {
        output->setState(OutputState::Error);
    } else {
        output->setState(OutputState::Offline);
    }
}

void RTMPOutput::obsOutputReconnect(void *data, calldata_t *)
{
    auto *output = static_cast<RTMPOutput*>(data);
    output->setState(OutputState::Connecting);
}

void RTMPOutput::obsOutputReconnectSuccess(void *data, calldata_t *)
{
    auto *output = static_cast<RTMPOutput*>(data);
    output->setState(OutputState::Online);
}

void RTMPOutput::setupAudioEncoder()
{
    obs_encoder_t *aencoder = obs_audio_encoder_create(
        config_->audioEncoderId.toUtf8().constData(),
        "audio_encoder",
        nullptr,
        0,
        nullptr);  // No hotkey data needed for audio encoder

    if (!aencoder) {
        obs_log(LOG_WARNING, "[RTMPOutput] Failed to create audio encoder");
        return;
    }

    // Configure audio encoder
    obs_data_t *asettings = obs_data_create();
    obs_data_set_int(asettings, "bitrate", config_->audioBitrateKbps);
    obs_encoder_update(aencoder, asettings);
    obs_data_release(asettings);

    // Attach encoder to output
    obs_output_set_audio_encoder(output_, aencoder, 0);

    obs_encoder_release(aencoder);
} 
