/*!
 * @file rtmp-output.h
 * @brief Declaration of the RTMPOutput class.
 *
 * @author <Developer> <Email Address>
 * @copyright Copyright (C) <Year> <Developer>
 * @license GNU General Public License v2 or later
 * @see https://www.gnu.org/licenses/
 */

#pragma once

#include "stream-output.h"
#include "rtmp-output-config.h"

class RTMPOutput : public StreamOutput {
    Q_OBJECT

public:
    explicit RTMPOutput(OBSConfigHelper *cfg, const QString &serviceName, const QString &serviceIconPath, QWidget *parent = nullptr);
    ~RTMPOutput() override;

    /// Initialize the output (call after construction)
    void initialize();

protected:
    void showConfig() override;
    void startStream() override;
    void stopStream() override;
    void loadConfig() override;
    void saveConfig() override;
    bool validateConfig() const override;

    virtual QString defaultServerUrl() const = 0;
    std::unique_ptr<RTMPOutputConfig> config_;

private:
    void setupAudioEncoder();
    void setupVideoEncoder(); 
    void setupRtmpOutput();
    void releaseOutput();

    // OBS output callbacks
    static void obsOutputStarted(void *data, calldata_t *cd);
    static void obsOutputStopped(void *data, calldata_t *cd);
    static void obsOutputReconnect(void *data, calldata_t *cd);
    static void obsOutputReconnectSuccess(void *data, calldata_t *cd);

    obs_output_t *output_{nullptr};
}; 
