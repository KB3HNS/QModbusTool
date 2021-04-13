/**
 * \file register_display.h
 * \brief Register set display window
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
 * The "RegisterDisplay" class is the base for all Modbus data pools.  This
 * class may be derived to form custom register set displays (such as writable
 * registers, indicators, and coils).  This also encludes a custom encoding
 * mechanism (ie non-modbus standard) to retrieve register metadata including
 * limits, defaults, data types, and a brief human-readable description.
 */

#ifndef REGISTER_DISPLAY_H
#define REGISTER_DISPLAY_H

//  c++ includes
#include <QLabel>  //  QLabel
#include <QScrollArea>  //  QScrollArea
#include <QGridLayout>  //  QGridLayout
#include <QStatusBar>  //  QStatusBar
#include <QGroupBox>  //  QGroupBox
#include <QSpinBox>  //  QSpinBox
#include <QPushButton>  //  QPushButton
#include <QString>  //  QString
#include <QTimer>  //  QTimer
#include <QDomElement>  //  QDomElement
#include <memory>  //  std::shared_ptr
#include <vector>  //  std::vector
#include <optional>  //  std::optional

// C includes
/* -none- */

// project includes
#include "base_dialog.h"  //  BaseDialog
#include "metadata_structs.h"  //  RegisterEncoding


/**
 * \brief Individual register set display.
 */
class RegisterDisplay : public BaseDialog
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
    RegisterDisplay(QWidget *parent, const quint16 base_reg, const quint16 count=32, const quint8 uid=0);

    /**
     * \brief Get the unique object name for save/restore
     * \note
     * All deriving classes must implement this method.
     *
     * @return Name of this object
     */
    virtual QString get_object_name() const;

    /**
     * \brief Save configuration parameters to XML node
     * @param node Store in node
     */
    virtual void save_configuration_parameters(QDomElement &node) const;

    /**
     * \brief Load / verify configuration parameters
     * @param node Read data from node
     * @return ``true`` if data is valid, ``false`` otherwise
     */
    virtual bool load_configuration_parameters(const QDomElement &node);

    virtual void set_metadata(std::shared_ptr<Metadata> metadata, const quint8 node) override;
    virtual void poll_register_set(ModbusThread *const engine) override;

protected:

    virtual void setupUi() override;

    /**
     * \brief Update register widget with new value.
     * @param index index \f(register number = start_reg + index)\f
     * @param value value to display
     */
    virtual void updateRegisterValue(const size_t index, const QString &value);

    /**
     * \brief Create a window-specific widget to display register data
     * @param index index within the window (0-count)
     * @param initial Initial call from setupUi
     * @return Widget to add to container
     */
    virtual QWidget* create_value_widget(const quint16 index, const bool initial);

    /**
     * \brief Decode modbus data to a displayable string.
     * @param value raw modbus value
     * @param encoding encoding to treat value as
     * @return String representation of register value
     */
    QString decode_register(const quint16 value, const RegisterEncoding encoding) const;

    /**
     * \brief Get the displayed value at an index
     * @param index register index in form
     * @return String representation of register value
     */
    virtual QString get_display_value(const size_t index) const;

    /**
      * \brief Get the label text for a register number
      * @param reg_number register number
      */
    [[nodiscard]] virtual QString get_register_number_text(const quint16 reg_number);

    /**
     * @brief Set the title text that should dectorate this window
     */
    virtual void set_title();

    quint16 m_starting_register; /**< First register polled by this window */
    quint16 m_count; /**< Number of registers polled */
    quint16 m_max_regs=0; /**< Maximum number of registers allowed */
    quint8 m_node; /**< Polls directed at this node/device ID */
    bool m_have_metadata = false; /**< Flag indicating that metadata has been polled */

    std::vector<QLabel*> m_register_labels; /**< List of register number labels */
    std::vector<QWidget*> m_register_values; /**< List of display values */
    std::vector<QLabel*> m_register_descriptions; /**< List of descriptions (from metadata) */

    /**
     * \var m_register_encoding
     * List of encoding attributes for each register
     * \sa RegisterEncoding
     */
    std::vector<RegisterEncoding> m_register_encoding;
    std::vector<quint16> m_raw_values; /**< List of raw values from protocol layer */

    QScrollArea *const m_scroll_area; /**< Main display scroll area */
    QWidget *const m_scroll_container; /**< Scroll area contents */
    QGridLayout *const m_scroll_layout; /**< Scroll area layout control */
    QStatusBar *const m_status; /**< Status notification area */

    QGroupBox *const m_control_box; /**< Group of configuration controls at top of scroll area */
    QGridLayout *const m_control_grid; /**< layout for control box \sa m_control_box */
    QSpinBox *const m_reg_select; /**< First register selection */
    QSpinBox *const m_node_select; /**< Node / device ID selection */
    QSpinBox *const m_quantity; /**< Number of registers selection */
    QPushButton *const m_apply_button; /**< Update window (also default) */
    QPushButton *m_refresh_button; /**< Refresh metadata */
    QPushButton *const m_save_button; /**< Save data to CSV table */

public slots:

    /**
     * \brief Signal on the refresh (update metadata) button clicked.
     * \note
     * Made public so the main window can trigger this as well.
     */
    void on_refresh_clicked();

    virtual void on_new_value(const quint16 reg, const quint16 value, const quint8 unit_id) override;
    virtual void on_exception_status(BaseDialog *requester, const QString exception) override;

protected slots:

    /**
     * \brief Signal on the apply (update configuration) button clicked.
     */
    void on_apply_clicked();

    /**
     * \brief Signal on the "save data" button click.
     */
    void on_save_clicked();

private slots:

    /**
     * \brief Signal on the status bar update timer expired.
     */
    void on_status_timer_timeout();

private:

    /**
     * \brief Save register data to CSV file
     * @param path absolute path and file name to save to
     */
    bool save_register_set(const QString &path);

    QTimer *const m_status_timer;
    bool m_meta_in_process = false;
};


#endif // REGISTER_DISPLAY_H
