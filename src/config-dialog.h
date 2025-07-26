/*!
 * @file config-dialog.h
 * @brief Declaration of the ConfigDialog class.
 *
 * @author <Developer> <Email Address>
 * @copyright Copyright (C) <Year> <Developer>
 * @license GNU General Public License v2 or later
 * @see https://www.gnu.org/licenses/
 */

#pragma once

#include "obs-config-helper.h"
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(OBSConfigHelper *cfg, QWidget *parent = nullptr);

protected:
    void accept() override;
    void reject() override;

private slots:
    void onLoad();

private:
    void loadFromCfg();
    void saveToCfg();

    static constexpr const char *kSec = "General";

    OBSConfigHelper *cfg_;
    QLineEdit *txt_;
    QSpinBox *num_;
    QComboBox *opt_;
};

