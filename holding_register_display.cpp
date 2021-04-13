/**
 * \file holding_register_display.cpp
 * \brief Holding Registers window
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
#include <QMessageBox>  //  QMessageBox
#include <QLineEdit>  //  QLineEdit
#include <chrono>  //  std::chrono::seconds

// C includes
/* -none- */

// project includes
#include "holding_register_display.h"  //  local include


HoldingRegisterDisplay::HoldingRegisterDisplay(QWidget *parent, const quint16 base_reg, const quint16 count, const quint8 uid)
        :RegisterDisplay(parent, base_reg, count, uid),
          m_activity_timer{new QTimer(this)}
{
    m_activity_timer->setInterval(std::chrono::seconds(8));
    m_activity_timer->setSingleShot(true);
    connect(m_activity_timer, &QTimer::timeout, this, &HoldingRegisterDisplay::on_timer_expired);
}


void HoldingRegisterDisplay::updateRegisterValue(const size_t index, const QString &value)
{
    if (!m_activity_timer->isActive() || int(index) != m_active_index) {
        auto widget = dynamic_cast<QLineEdit*>(m_register_values[index]);
        widget->setText(value);
    }
}


QWidget* HoldingRegisterDisplay::create_value_widget(const quint16 index, const bool initial)
{
    static_cast<void>(initial);
    auto widget = new QLineEdit(m_scroll_area);
    connect(widget, &QLineEdit::textEdited, this, [=]() {
        on_register_textEdited(index);
    });
    connect(widget, &QLineEdit::returnPressed, this, [=]() {
       on_register_returnPressed(index);
    });
    connect(widget, &QLineEdit::editingFinished, this, [=]() {
        on_register_editingFinished(index);
    });
    connect(widget, &QLineEdit::destroyed, this, [=](QWidget *w=nullptr) {
        static_cast<void>(w);
        on_register_destroyed(widget, index);
    });
    return widget;
}


void HoldingRegisterDisplay::on_register_textEdited(const quint16 index)
{
    auto widget = dynamic_cast<QLineEdit*>(m_register_values[index]);
    m_active_index=int(index);
    widget->setStyleSheet("background-color: #FFF0F0;");
    m_activity_timer->start();
}


void HoldingRegisterDisplay::on_register_returnPressed(const quint16 index)
{
    on_register_editingFinished(index);
    const auto display_value = get_display_value(index);
    auto encoded_regs = encode_register(display_value, m_register_encoding[index]);

    if (encoded_regs.size() == 0) {
        auto error_box = QMessageBox(this);
        error_box.setIcon(QMessageBox::Critical);
        error_box.setStandardButtons(QMessageBox::Ok);
        error_box.setText(tr("Unable to parse input"));
        error_box.setInformativeText(
                    tr("Error parsing '%1' : values not updated").arg(display_value));
        error_box.exec();
    } else {
        WriteRequest req{};
        req.first_register = m_starting_register + index;
        req.node = m_node;
        req.values = std::move(encoded_regs);
        req.requester = this;
        emit write_requested(req);
    }
}


void HoldingRegisterDisplay::on_register_editingFinished(const quint16 index)
{
    auto widget = dynamic_cast<QLineEdit*>(m_register_values[index]);
    widget->setStyleSheet("background-color: #FFF;");
    m_active_index=-1;
    m_activity_timer->stop();
}


void HoldingRegisterDisplay::on_register_destroyed(QWidget *register_widget, const quint16 index)
{
    static_cast<void>(register_widget);
    static_cast<void>(index);
}


void HoldingRegisterDisplay::on_timer_expired()
{
    if (m_active_index >= 0) {
        on_register_editingFinished(quint16(m_active_index));
    }
}


QString HoldingRegisterDisplay::get_object_name() const
{
    return "HoldingRegisterDisplay";
}


bool HoldingRegisterDisplay::load_configuration_parameters(const QDomElement &node)
{
    auto reg = node.attribute("register").toUInt();
    if (reg < 40001 || reg >= 50000) {
        return false;
    }

    return RegisterDisplay::load_configuration_parameters(node);
}


QString HoldingRegisterDisplay::get_display_value(const size_t index) const
{
    const auto widget = dynamic_cast<QLineEdit*>(m_register_values[index]);
    return widget->text();
}


std::vector<quint16> HoldingRegisterDisplay::encode_register(
        const QString &value, const RegisterEncoding encoding) const
{
    std::vector<quint16> raw_values;
    bool ok = true;
    switch (encoding) {
    case RegisterEncoding::ENCODING_NONE:
    case RegisterEncoding::ENCODING_UINT16:
    case RegisterEncoding::ENCODING_UNKNOWN:
    case RegisterEncoding::ENCODING_USER:
        raw_values.push_back(quint16(value.toInt(&ok)));
        break;

    case RegisterEncoding::ENCODING_BITS:
        raw_values.push_back(quint16(value.mid(2).toInt(&ok, 16)));
        break;

    case RegisterEncoding::ENCODING_SIGNED_BYTES: {
            const auto s = value.split(',');
            bool oka = false;
            bool okb = false;
            if (s.size() == 2) {
                const auto ia = qint8(s[0].toInt(&ok));
                const auto ib = qint8(s[1].toInt(&ok));
                auto v16 = quint16(static_cast<quint8>(ia) << 8);
                v16 |= static_cast<quint8>(ib);
                raw_values.push_back(v16);
            }
            ok = (oka && okb);
        } break;

    case RegisterEncoding::ENCODING_BYTES: {
            const auto s = value.split(',');
            bool oka = false;
            bool okb = false;
            if (s.size() == 2) {
                const auto ua = quint8(s[0].toInt(&ok));
                const auto ub = quint8(s[1].toInt(&ok));
                auto v16 = quint16(ua << 8);
                v16 |= ub;
                raw_values.push_back(v16);
            }
            ok = (oka && okb);
        } break;

    case RegisterEncoding::ENCODINT_INT16: {
            const auto ival = qint16(value.toInt(&ok));
            raw_values.push_back(static_cast<quint16>(ival));
        } break;
    }

    if (ok) {
        return raw_values;
    }

    return {};
}


void HoldingRegisterDisplay::setupUi()
{
    RegisterDisplay::setupUi();
    m_apply_button->setDefault(false);
    m_apply_button->setAutoDefault(false);
    if (nullptr != m_refresh_button) {
        m_refresh_button->setDefault(false);
        m_refresh_button->setAutoDefault(false);
    }
    m_save_button->setDefault(false);
    m_save_button->setAutoDefault(false);
}
