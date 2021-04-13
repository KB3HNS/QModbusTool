/**
 * \file configure_trend_line.cpp
 * \brief Trend configuration dialog box
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
#include <QColorDialog>  //  QColorDialog
#include <QMessageBox>  //  QMessageBox
#include <QStringBuilder>  //  operator%

// C includes
/* -none- */

// project includes
#include "configure_trend_line.h"  //  local include
#include "exceptions.h"  //  AppException


ConfigureTrendLine::ConfigureTrendLine(TrendWindow *const parent) :
    BaseDialog(parent, false),
    m_ui{new Ui::ConfigureTrendDialog()},
    m_display_color{Qt::black},
    m_parent{parent},
    m_trend{nullptr}
{
    m_ui->setupUi(this);
}


ConfigureTrendLine::ConfigureTrendLine(TrendLine *const parent) :
    ConfigureTrendLine(parent->m_parent)
{
    m_trend = parent;
    m_display_color = parent->m_pen_color;
}


void ConfigureTrendLine::setupUi()
{
    update_color_labels();

    add_icon_to_button(m_ui->Accept, QStyle::SP_DialogApplyButton);
    add_icon_to_button(m_ui->Delete, QStyle::SP_BrowserStop);
    add_icon_to_button(m_ui->Cancel, QStyle::SP_DialogCloseButton);

    if (nullptr == m_trend) {
        m_ui->Accept->setText(tr("Create"));
        m_ui->Delete->setHidden(true);
    } else {
        m_ui->RegEdit->setText(QString::number(m_trend->m_reg_number));
        m_ui->RegEdit->setEnabled(false);
        m_ui->NodeEdit->setValue(int(m_trend->m_device_id));
        m_ui->NodeEdit->setEnabled(false);
        m_ui->SignedEdit->setChecked(m_trend->m_signed_value);
        m_ui->MultBox->setText(QString::number(m_trend->m_mult));
        m_ui->OffsetBox->setText(QString::number(m_trend->m_offset));
    }
}


void ConfigureTrendLine::on_Accept_pressed()
{
    bool ok;
    const auto reg = m_ui->RegEdit->text().toInt(&ok);
    QStringList error_text;
    if (!ok || (reg <= 0) || (reg >= 50000) || (reg >= 20000 && reg <= 30000)) {
        error_text << tr("Illegal register number");
    }

    const auto m = m_ui->MultBox->text().toDouble(&ok);
    if (!ok) {
        error_text << tr("Illegal multiply value");
    }

    const auto b = m_ui->OffsetBox->text().toDouble(&ok);
    if (!ok) {
        error_text << tr("Illegal offset value");
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
        return;
    }

    if (nullptr != m_trend) {
        m_trend->configure(m, b, m_ui->SignedEdit->isChecked());
        m_trend->set_color(m_display_color);
    }

    accept();
}


void ConfigureTrendLine::on_Cancel_pressed()
{
    reject();
}


void ConfigureTrendLine::on_Delete_pressed()
{
    reject();
    m_parent->remove_trend(quint32(*m_trend));
}


void ConfigureTrendLine::on_ColorButton_pressed()
{
    auto dlg = QColorDialog(m_display_color, this);
    if (dlg.exec() != 0) {
        m_display_color = dlg.selectedColor();
        update_color_labels();
    }
}


void ConfigureTrendLine::update_color_labels()
{
    m_ui->ColorText->setText(m_display_color.name());
    m_ui->ColorSample->setStyleSheet("QLabel {"
                                     "color: " % m_display_color.name() %
                                     "; background-color: #FFFFFF;}");
}


TrendLine* ConfigureTrendLine::create_trend()
{
    if (nullptr != m_trend) {
        throw AppException(tr("Error, creating duplicate TrendLine"));
    }

    const auto m = m_ui->MultBox->text().toDouble();
    const auto b = m_ui->OffsetBox->text().toDouble();
    const auto node = m_ui->NodeEdit->value();
    const auto reg = m_ui->RegEdit->text().toInt();
    auto trend = new TrendLine(m_parent, quint16(reg), quint8(node));
    trend->configure(m, b, m_ui->SignedEdit->isChecked());
    trend->set_color(m_display_color);
    m_trend = trend;

    return trend;
}


ConfigureTrendLine::operator quint32() const
{
    return m_parent->get_key(quint16(m_ui->RegEdit->text().toUInt()),
                             quint8(m_ui->NodeEdit->value()));
}


ConfigureTrendLine::~ConfigureTrendLine()
{
    delete m_ui;
}
