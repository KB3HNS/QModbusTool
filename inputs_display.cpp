/**
 * \file inputs_display.cpp
 * \brief [Digital] Inputs display window
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
/* -none- */

// C includes
/* -none- */

// project includes
#include "inputs_display.h"  //  local include


InputsDisplay::InputsDisplay(QWidget *parent, const quint16 base_reg, const quint16 count, const quint8 uid)
    : RegisterDisplay(parent, base_reg, count, uid)
{
}


bool InputsDisplay::load_configuration_parameters(const QDomElement &node)
{
    auto reg = node.attribute("register").toUInt();
    if (reg <= 10000 || reg >= 20000) {
        return false;
    }

    return RegisterDisplay::load_configuration_parameters(node);
}


QString InputsDisplay::get_object_name() const
{
    return "InputsDisplay";
}


void InputsDisplay::updateRegisterValue(const size_t index, const QString &value)
{
    static_cast<void>(value);
    const auto wvalue = (m_raw_values[index] > 0 ? QString("1") : QString("0"));
    RegisterDisplay::updateRegisterValue(index, wvalue);
}
