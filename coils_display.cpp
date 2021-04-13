/**
 * \file coils_display.cpp
 * \brief Coils display window
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
#include <QCheckBox>  //  QCheckBox
#include <QStringBuilder>  //  operator%

// C includes
/* -none- */

// project includes
#include "coils_display.h"  //  local include


CoilsDisplay::CoilsDisplay(QWidget *parent, const quint16 base_reg, const quint16 count, const quint8 uid)
    :RegisterDisplay(parent, base_reg, count, uid),
      m_remote_state()
{

}


void CoilsDisplay::updateRegisterValue(const size_t index, const QString &value)
{
    static_cast<void>(value);
    auto widget = dynamic_cast<QCheckBox*>(m_register_values[index]);
    auto bvalue = (m_raw_values[index] > 0);
    m_remote_state[quint16(index + m_starting_register)] = bvalue;
    widget->setChecked(bvalue);
}


QWidget* CoilsDisplay::create_value_widget(const quint16 index, const bool initial)
{
    static_cast<void>(initial);
    auto widget = new QCheckBox(this);
    connect(widget, &QCheckBox::stateChanged, this, [=](int state) {
        static_cast<void>(state);
        on_checkbox_checked(index);
    });
    connect(widget, &QCheckBox::destroyed, this, [=](QWidget *w=nullptr) {
        static_cast<void>(w);
        on_register_destroyed(widget, index);
    });
    return widget;
}


void CoilsDisplay::on_checkbox_checked(quint16 index)
{
    const auto checkbox = dynamic_cast<QCheckBox*>(m_register_values[index]);
    const auto a = m_remote_state[index + m_starting_register];
    const auto b = checkbox->isChecked();
    if (a != b) {
        WriteRequest req{};
        req.first_register = m_starting_register + index;
        req.node = m_node;
        req.values = {quint16(b)};
        req.requester = this;
        emit write_requested(req);
    }
}


void CoilsDisplay::on_register_destroyed(QWidget *register_widget, const quint16 index)
{
    static_cast<void>(register_widget);
    static_cast<void>(index);
}


QString CoilsDisplay::get_object_name() const
{
    return "CoilsDisplay";
}


bool CoilsDisplay::load_configuration_parameters(const QDomElement &node)
{
    const auto reg = node.attribute("register").toUInt();
    if (reg >= 10000) {
        return false;
    }

    return RegisterDisplay::load_configuration_parameters(node);
}


QString CoilsDisplay::get_display_value(const size_t index) const
{
    auto widget = dynamic_cast<QCheckBox*>(m_register_values[index]);
    return (widget->isChecked() ? "1" : "0");
}

QString CoilsDisplay::get_register_number_text(const quint16 reg_number)
{
    const auto base_text = RegisterDisplay::get_register_number_text(reg_number);
    const auto padding = QString(5 - base_text.size(), QChar('0'));
    return padding % base_text;
}
