/**
 * \file scheduler.h
 * \brief Poll request scheduler
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
 * Implement a multi-tier scheduling mechanism.  Although Modbus/TCP allows
 * for queued requests libmodbus doesn't support this feature.  Therefore this
 * works as a single request-response mechanism.  All requests are one-shot and
 * require requests to re-enqueue any periodic polls.  A complete set of thread-
 * safe signals are provided for key events including register data dispatch
 * which is intended for situations where there are multiple consumers of a
 * register data point.  Be aware that the interface is not thread safe, only
 * the signals and slots.  Interface APIs are only to be called from the main\
 * (window) thread.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

//  c++ includes
#include <chrono>  //  std::chrono::milliseconds
#include <deque>  //  std::deque
#include <QObject>  //  QObject
#include <QPair>  //  QPair

// C includes
/* -none- */

// project includes
#include "write_event.h"  //  WriteRequest
#include "register_display.h"  //  RegisterDisplay
#include "modbusthread.h"  //  ModbusThread
#include "metadata_structs.h"  //  WindowMetadataRequest


/**
 * \brief Enumeration of the valid poll request sources
 */
enum PollAction : int {
    POLLING_INACTIVE,
    POLLING_WRITE,
    POLLING_METADATA,
    POLLING_READ,
    POLLING_DEVID
};


/**
 * @brief Meanings associated with the "system" register (0)
 */
enum SystemRegister : quint16 {
    DEVICE_ID_POLL_COMPLETE=0,
    CUSTOM_POLL_COMPLETE,
    POLL_METADATA_COMPLETE,
    WRITE_REQUEST_COMPLETE,
    SYSTEM_CONNECTED,
    SYSTEM_DISCONNECTED
};


/**
 * \brief Modbus poll scheduler
 */
class Scheduler : public QObject
{
    Q_OBJECT

public:

    /**
     * \brief constructor
     * @param parent parent QObject owner
     */
    Scheduler(QObject *parent);

    /**
     * \brief Initiate modbus with connection
     * @param engine Modbus connection
     * @param timeout poll timeout
     */
    void start_modbus(ModbusThread *const engine, const std::chrono::milliseconds timeout);

    /**
     * \brief Immediately release all modbus resources and stop polling.
     */
    void stop_modbus();

    /**
     * \brief Enqueue a register screen to have registers polled.
     * \note
     * RegisterDisplay objects are treated as the primary source of poll requests.
     *
     * @param source window or object that has poll data requests
     */
    void enqueue_request(BaseDialog *const source);

    /**
     * \brief Immediately release all references in all queues to a specified
     *        data object (screen)
     * @param screen reference to be removed
     */
    void remove_reference(BaseDialog *const screen);

    /**
     * \brief Get the overall success and error poll counts.
     * @return pair of success and error counts since this connection began
     */
    [[nodiscard]] QPair<quint64, quint64> get_counts() const;

    /**
     * \brief Check to see if the scheduler has an active request.
     * @param requester [out] update with the current request source
     * @return ``true`` if active, ``false`` otherwise
     */
    [[nodiscard]] bool get_active(BaseDialog* &requester) const;

signals:

    /**
     * \brief Emit new reqister data.
     * \note
     * regnumber 0 has special meaning, see enum above
     *
     * @param regnumber register number (eg: 40001, 2, 10003, 30042)
     * @param value register value
     * @param device_id device id / node polled
     */
    void new_register_data(const quint16 regnumber, const quint16 value, const quint8 device_id);

    /**
     * \brief Emit when the primary (enqueue_request) queue becomes empty.
     */
    void polling_complete();

    /**
     * \brief Emit when a poll returns an exception reqponse
     * \note
     * requesster may be null if the request did not originate from a
     * data source (ie reference removed or another source).
     *
     * @param requester The source of the request (see note)
     * @param exception Exception text
     */
    void poll_exception(BaseDialog *const requester, const QString exception);

public slots:

    /**
     * \brief Signal to request a modbus write.
     * @param request request structure
     */
    void modbus_on_write_request(WriteRequest request);

    /**
     * \brief Signal to request a modbus non-standard poll sequence.
     * @param request_sequence description of metadata to read
     */
    void modbus_on_poll_meta(WindowMetadataRequest request_sequence);

protected slots:

    /**
     * \brief Signal from modbus thread on a poll exception.
     * @param error_code modbus exception code
     */
    void modbus_on_error(const int error_code);

    /**
     * \brief Signal from modbus thread on a poll complete.
     */
    void modbus_on_data();

    /**
     * \brief Signal from timer when a poll request has timed out.
     */
    void modbus_on_timer_expired();

protected:

    /**
     * \brief Main logic to determine and send next poll
     */
    void figure_next();

    std::deque<WriteRequest> m_write_requests; /**< list of pending write requests */

    /**
     * \var m_meta_requests
     *  list of pending read metadata requests
     */
    std::deque<WindowMetadataRequest> m_meta_requests;

    /**
     * \var m_standard_requests
     *  list of pending write requests
     */
    std::deque<BaseDialog*> m_standard_requests;
    ModbusThread *m_polling_thread=nullptr; /**< Pointer to thread (when connected) */

    /**
     * \var m_current_action
     *  Current polling state
     */
    PollAction m_current_action=PollAction::POLLING_INACTIVE;

private:

    /* individaul poll generators */
    void poll_write_request();
    bool poll_meta_request();
    bool poll_read_request();
    void poll_devid_request();
    void poll_response_metadata();

    quint64 m_poll_count=0;
    quint64 m_error_count=0;
    QTimer *const m_modbus_timer;
    bool m_active=false;
    BaseDialog *m_current_request=nullptr;
};

#endif // SCHEDULER_H
