/**
 * \file configure_trend.cpp
 * \brief Trend graph configuration dialog box
 * \copyright
 * 2021 Andrew Buettner (ABi)
 *
 * \section LICENSE
 *
 * QModbusTool - A QT Based Modbus Client
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


//  c++ includes
#include <QPushButton>  //  QPushButton
#include <QMessageBox>  //  QMessageBox
#include <QStringList>  //  QStringList

// C includes
/* -none- */

// project includes
#include "configure_trend.h"  //  local include


namespace {
    const auto g_max_points = 1000000;  //  some arbitrary limit.
}


ConfigureTrend::ConfigureTrend(QWidget *const parent,
                               const double min,
                               const double max,
                               const int n_points) :
        BaseDialog(parent, false),
        m_ui{new Ui::ConfigureTrend()},
        m_initial_points{n_points},
        m_initial_min{min},
        m_initial_max{max},
        m_first_edit{true}
{
    m_ui->setupUi(this);
    m_ui->MinInput->setText(QString::number(min));
    m_ui->MaxInput->setText(QString::number(max));
}


void ConfigureTrend::setupUi()
{
    m_ui->HistoryInput->setText(QString::number(m_initial_points));
    QPushButton *btn1 = m_ui->buttonBox->button(QDialogButtonBox::Cancel);
    add_icon_to_button(btn1, QStyle::SP_DialogCloseButton);

    QPushButton *btn2 = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    add_icon_to_button(btn2, QStyle::SP_DialogApplyButton);
}


void ConfigureTrend::on_HistoryInput_textEdited()
{
    if (m_first_edit) {
        m_first_edit = false;
        auto msg_box = QMessageBox(this);
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        msg_box.setText(tr("Change history size"));
        msg_box.setInformativeText(tr(
               "Notice:\n"
               "Changing the history size will clear currently captured data."));
        if (msg_box.exec() != QMessageBox::Ok) {
            m_ui->HistoryInput->setText(QString::number(m_initial_points));
        }
    }
}


std::pair<double, double> ConfigureTrend::get_min_max() const
{
    bool okmin, okmax;
    auto min = m_ui->MinInput->text().toDouble(&okmin);
    if (!okmin) {
        min = m_initial_min;
    }

    auto max = m_ui->MaxInput->text().toDouble(&okmax);
    if (!okmax) {
        max = m_initial_max;
    }

    if (min >= max) {
        min = m_initial_min;
        max = m_initial_max;
    }

    return { min, max };
}


std::optional<int> ConfigureTrend::get_num_points() const
{
    bool ok;
    const auto new_points = m_ui->HistoryInput->text().toInt(&ok);
    if ((m_initial_points == new_points) || !ok || (new_points < 3)) {
        return std::optional<int>();
    }

    return new_points;
}


void ConfigureTrend::accept()
{
    QStringList error_text;
    bool okmin, okmax;
    const auto min = m_ui->MinInput->text().toDouble(&okmin);
    if (!okmin) {
        error_text << tr("Invalid minimum value specified.");
    }

    const auto max = m_ui->MaxInput->text().toDouble(&okmax);
    if (!okmax) {
        error_text << tr("Invalid maximum value specified.");
    }

    if (okmin && okmax && (min >= max)) {
        error_text << tr("Max value must be greater than min value.");
    }

    bool ok;
    const auto points = m_ui->HistoryInput->text().toInt(&ok);
    if (!ok) {
        error_text << tr("Invalid number of points specified.");
    } else if (points < 3 || points > g_max_points) {
        error_text << tr("Points must be between 3 and %1").arg(g_max_points);
    }

    if (error_text.size() > 0) {
        auto error_box = QMessageBox(this);
        error_box.setIcon(QMessageBox::Critical);
        error_box.setStandardButtons(QMessageBox::Ok);
        error_box.setText(tr("Invalid trend configuration specified"));
        error_box.setInformativeText(tr("Errors were detected"));
        error_box.setWindowTitle(tr("Invalid Configuration"));
        error_box.setDetailedText(error_text.join('\n'));
        error_box.exec();
    } else {
        BaseDialog::accept();
    }
}


ConfigureTrend::operator bool() const
{
    return m_ui->DynamicCheck->isChecked();
}


ConfigureTrend::~ConfigureTrend()
{
    delete m_ui;
}
