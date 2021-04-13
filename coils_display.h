/**
 * \file coils_display.h
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
 *
 * \section DESCRIPTION
 *
 * Derivitave of RegisterDisplay for interacting with Coils (0,000 series
 * Modbus registers).
 */

#ifndef COILSDISPLAY_H
#define COILSDISPLAY_H

//  c++ includes
#include <unordered_map>  //  std::unordered_map

// C includes
/* -none- */

// project includes
#include "register_display.h"  //  RegisterDisplay


/**
 * \brief Holding Registers windows
 */
class CoilsDisplay : public RegisterDisplay
{
    Q_OBJECT

public:
    /**
     * \brief constructor
     * @param parent parent QObject owner
     * @param base_reg first register number
     * @param count number of registers to poll
     * @param uid device ID, unit ID, node of server to poll
     */
    CoilsDisplay(QWidget *parent, const quint16 base_reg, const quint16 count=32, const quint8 uid=0);

    virtual QString get_object_name() const override;
    virtual bool load_configuration_parameters(const QDomElement &node) override;

protected:
    virtual void updateRegisterValue(const size_t index, const QString &value) override;
    virtual QWidget* create_value_widget(const quint16 index, const bool initial) override;
    virtual QString get_display_value(const size_t index) const override;
    [[nodiscard]] virtual QString get_register_number_text(const quint16 reg_number) override;

private slots:
    /**
     * \brief Signal when a user has toggled a checkbox (write request).
     * @param index register index in window
     */
    void on_checkbox_checked(quint16 index);

    /**
     * \brief Signal that a control has been destroted.
     * @param register_widget widget reference
     * @param index register index in window
     */
    void on_register_destroyed(QWidget *register_widget, const quint16 index);

private:
    std::unordered_map<quint16, bool> m_remote_state;

};

#endif // COILSDISPLAY_H
