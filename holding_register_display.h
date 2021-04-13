/**
 * \file holding_register_display.h
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
 *
 * \section DESCRIPTION
 *
 * Derivitave of Register Display for interacting with the Holding Registers
 * (40,000 series)
 */

#ifndef HOLDINGREGISTERDISPLAY_H
#define HOLDINGREGISTERDISPLAY_H

//  c++ includes
#include <QTimer>  //  QTimer
#include <vector>  //  std::vector

// C includes
/* -none- */

// project includes
#include "register_display.h"  //  RegisterDisplay


/**
 * \brief Holding Registers windows
 */
class HoldingRegisterDisplay : public RegisterDisplay
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
    HoldingRegisterDisplay(QWidget *parent, const quint16 base_reg, const quint16 count=32, const quint8 uid=0);

    virtual QString get_object_name() const override;
    virtual bool load_configuration_parameters(const QDomElement &node) override;

protected:

    /**
     * \brief Encode display string to raw unsigned 16 value(s)
     * @param value String value
     * @param encoding Register's encoding
     * @return list of values to build as a write request
     */
    [[nodiscard]] virtual std::vector<quint16> encode_register(const QString &value,
                                                               const RegisterEncoding encoding) const;

    virtual void updateRegisterValue(const size_t index, const QString &value) override;
    virtual QWidget* create_value_widget(const quint16 index, const bool initial) override;
    virtual QString get_display_value(const size_t index) const override;
    virtual void setupUi() override;

private slots:
    /**
     * \brief Signal that a register text box is active for editing.
     * @param index register index in window
     */
    void on_register_textEdited(const quint16 index);

    /**
     * \brief Signal that enter has been pressed on a text box (write request).
     * @param index register index in window
     */
    void on_register_returnPressed(const quint16 index);

    /**
     * \brief Signal that a text box is no longer being edited.
     * @param index register index in window
     */
    void on_register_editingFinished(const quint16 index);

    /**
     * \brief Signal that a control has been destroted.
     * @param register_widget widget reference
     * @param index register index in window
     */
    void on_register_destroyed(QWidget *const register_widget, const quint16 index);

    /**
     * \brief Signal that the edit idle timer has expired.
     */
    void on_timer_expired();

private:
    int m_active_index=-1;
    QTimer *const m_activity_timer;

};

#endif // HOLDINGREGISTERDISPLAY_H
