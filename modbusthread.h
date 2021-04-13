/**
 * \file modbusthread.h
 * \brief Modbus thread container
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
 * Modbus protocol functionality is abstracted (somewhat) from the display and
 * takes place in a separate thread because all libmodbus transactions are
 * treated as blocking calls.
 */

#ifndef MODBUSTHREAD_H
#define MODBUSTHREAD_H

//  c++ includes
#include <QThread>  //  QThread
#include <QMutex>  //  QMutex
#include <QWaitCondition>  //  QWaitCondition
#include <QString>  //  QString

// C includes
#include <modbus/modbus.h>  //  modbus_t

// project includes
/* -none- */


/**
 * \brief Modbus communication thread
 */
class ModbusThread : public QThread
{
    Q_OBJECT

public:

    /**
     * \brief constructor
     * @param parent parent QObject owner
     * @param host host to be resolved and connected to
     * @param port destination port
     */
    ModbusThread(QObject *parent, const QString &host, const quint16 port);

    /**
     * \brief Close and exit thread.
     * \note
     * Blocking call
     */
    void close();

    /**
     * \brief Issue a modbus read request
     * @param first_reg first register number (0 = device id)
     * @param num_regs number of registers to read
     * @param uid Unit ID / Node to poll
     */
    void modbus_request(const quint16 first_reg, const quint16 num_regs, const quint8 uid);

    /**
     * \brief Issue a modbus write request
     * @param first_reg first register number
     * @param regs_to_write list of values to be written
     * @param uid Unit ID / Node to poll
     */
    void modbus_request(const quint16 first_reg, std::vector<quint16> &&regs_to_write, const quint8 uid);

    /**
     * \brief Issue a raw modbus PDU
     * @param pdu Modbus raw PDU to send
     * @param length length of PDU
     * @param fc function code
     * @param uid Unit ID / Node to poll
     */
    void modbus_request(const quint8 *pdu, const quint8 length, const qint8 fc, const quint8 uid);

    /**
     * \brief Obtain results from a previous modbus transaction.
     * \note
     * vector will contain a character array (quint8) if the request was
     * device ID or a custom response.  Values promoted from quint8 to
     * quint16.  This call results in the value being std::move'd from
     * internal storage and can be made only once for each complete
     * signal.
     *
     * @return register list
     */
    [[nodiscard]] std::vector<quint16> modbus_result();

    /**
     * \brief Get the starting register for the most recent request call.
     * \note
     * This call and the next can't be const because they invoke lock.
     * \sa get_unit_id
     * @return register number (0 = id, 0xffff = custom)
     */
    [[nodiscard]] quint16 get_start_reg();

    /**
     * \brief Get the node (unit id) that the most recent request was sent to.
     * @return device / node / unit ID
     */
    [[nodiscard]] quint8 get_unit_id();

    virtual void run() override;

    ~ModbusThread() override;

signals:

    /**
     * \brief Emit poll exception.
     * @param error_code Modbus error code reported
     */
    void modbus_error(const int error_code);

    /**
     * \brief Emit poll complete.
     */
    void complete();

private:

    /**
     * \brief Assemble a write multiple registers request
     * @return result from modbus call
     */
    int do_write_request();

    /**
     * \brief Consolidate the logic for custom requests (Modbus/TCP).
     * @return result code
     */
    int do_custom_request_tcp();

    const QString m_host;
    const quint16 m_port;
    modbus_t *m_ctx=nullptr;
    QMutex m_mx;
    QWaitCondition m_cond;
    bool m_quit=false;
    bool m_write_request=false;
    const quint8 *m_raw_request=nullptr;

    std::vector<quint16> m_regs;
    quint16 m_reg_number=0;
    quint16 m_count=0;
    quint8 m_node=0;
};


#endif // MODBUSTHREAD_H
