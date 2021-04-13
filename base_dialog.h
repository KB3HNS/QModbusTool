/**
 * \file base_dialog.h
 * \brief Base class for most screens
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
 * This class acts as a base class for many of the modal windows used in the
 * project.  It specifically provides some basic definitions needed to interact
 * with the scheduler as well as delayed widget population.
 */

#ifndef BASE_DIALOG_H
#define BASE_DIALOG_H

//  c++ includes
#include <QDialog>  //  QDialog
#include <QResizeEvent>  //  QResizeEvent
#include <QVBoxLayout>  //  QVBoxLayout
#include <QAbstractButton>  //  QAbstractButton
#include <QStyle>  //  QStyle

// C includes
/* -none- */

// project includes
#include "write_event.h"  //  WriteRequest
#include "metadata_structs.h"  //  WindowMetadataRequest
#include "modbusthread.h"  //  ModbusThread


/**
 * \brief Modal window
 */
class BaseDialog : public QDialog
{
    Q_OBJECT

public:

    /**
     * \brief constructor
     * @param parent parent QObject owner
     * @param create_layout set ``false`` to omit creating the top QVBoxLayout
     */
    BaseDialog(QWidget *const parent, const bool create_layout=true);

    void add_icon_to_button(QAbstractButton *const button,
                            const QStyle::StandardPixmap icon_name) const;

    /**
     * \brief Set metadata callback from scheduler
     * @param metadata Metadata returned from poller
     * @param node Node (slave ID) that was polled
     */
    virtual void set_metadata(std::shared_ptr<Metadata> metadata, const quint8 node);

    /**
     * \brief Callback from scheduler to poll register data (1-poll)
     * @param engine Modbus connection
     */
    virtual void poll_register_set(ModbusThread *const engine);

public slots:

    /**
     * \brief Signal to update a register value.
     * @param reg register number
     * @param value raw register value
     * @param unit_id node / unit ID polled
     */
    virtual void on_new_value(const quint16 reg, const quint16 value, const quint8 unit_id);

    /**
     * \brief Signal that a modbus exception occurred.
     * @param requester request source associated with the exception
     * @param exception exception text
     */
    virtual void on_exception_status(BaseDialog *requester, const QString exception);

signals:

    /**
     * \brief Emit that the window has been closed.
     * @param window this window
     */
    void window_closed(BaseDialog *window);

    /**
     * \brief Emit that the window has been shown for the first time.
     * @param window this window
     */
    void window_first_display(BaseDialog *window);

    /**
     * \brief Emit data write request.
     * @param req write request structure
     */
    void write_requested(WriteRequest req);

    /**
     * \brief Emit read metadata request.
     * @param req read metadata request structure
     */
    void metadata_requested(WindowMetadataRequest req);

protected:

    /**
     * \brief Initialize the UI components.
     */
    virtual void setupUi() = 0;

    virtual void showEvent(QShowEvent *evt) override;
    virtual void resizeEvent(QResizeEvent *evt) override;
    virtual void closeEvent(QCloseEvent *evt) override;

    QVBoxLayout *const m_top_layout; /**< Window layout control */
    bool m_debug_resize = false;  /*< derived classes may change this to print window sizes */

private:
    bool m_first_display;
    bool m_emit_close;
};


#endif // BASE_DIALOG_H
