/**
 * \file modbusthread.cpp
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
 */

//  c++ includes
/* -none- */

// C includes
#include <sys/socket.h>  //  recv

// project includes
#include "modbusthread.h"  //  local include


ModbusThread::ModbusThread(QObject *parent, const QString &host, const quint16 port)
        :QThread(parent),
          m_host(host),
          m_port(port),
          m_mx(),
          m_cond(),
          m_regs()
{
    connect(this, &ModbusThread::finished, this, &ModbusThread::deleteLater);
}


ModbusThread::~ModbusThread()
{
    if (nullptr != m_ctx) {
        modbus_free(m_ctx);
        m_ctx=nullptr;
    }
}


void ModbusThread::run()
{
    bool exit_signal;
    m_ctx = modbus_new_tcp(m_host.toLocal8Bit().data(), int(m_port));
    if (nullptr == m_ctx) {
        m_mx.lock();
        m_quit = true;
        m_mx.unlock();
        emit modbus_error(errno);
        return;
    }

    if (modbus_connect(m_ctx) != 0) {
        m_mx.lock();
        m_quit = true;
        m_mx.unlock();
        emit modbus_error(errno);
        return;
    }

    emit complete();

    std::vector<uint8_t> bits;
    do {
        m_mx.lock();
        if (!m_quit) {
            m_cond.wait(&m_mx);
        }
        exit_signal = m_quit;
        int result;
        auto bit_process=false;
        m_regs.resize(size_t(m_count));

        //  This does not generate network traffic.
        if (nullptr == m_raw_request) {
            result=modbus_set_slave(m_ctx, int(m_node));
        } else {
            result=0;
        }

        if (exit_signal) {
            result=0;
        } else if (0 != result) {
            //  Don't do anything
        } else if (nullptr != m_raw_request) {
            result = do_custom_request_tcp();
        } else if (0 == m_count) {
            bits.resize(256);
            result = modbus_report_slave_id(m_ctx, 256, &bits[0]);
            bit_process=true;
            if (result > 0) {
                m_count=quint16(result);
                m_regs.resize(m_count);
            }
        } else if (m_write_request) {
            result = do_write_request();
        } else if (m_reg_number >= 1 && m_reg_number <= 9999) {
            bits.resize(size_t(m_count));
            result = modbus_read_bits(m_ctx, int(m_reg_number - 1), int(m_count), bits.data());
            bit_process=true;
        } else if (m_reg_number >= 10001 && m_reg_number <= 19999) {
            bits.resize(size_t(m_count));
            result = modbus_read_input_bits(m_ctx, int(m_reg_number - 10001), int(m_count), bits.data());
            bit_process=true;
        } else if (m_reg_number >= 30001 && m_reg_number <= 39999) {
            result = modbus_read_input_registers(m_ctx, int(m_reg_number - 30001), int(m_count), m_regs.data());
        } else if (m_reg_number >= 40001 && m_reg_number <= 49999) {
            result = modbus_read_registers(m_ctx, int(m_reg_number - 40001), int(m_count), m_regs.data());
        } else {
            emit modbus_error(2);  //  Illegal address
        }

        if (result < 0) {
            emit modbus_error(errno);
        } else {
            if (bit_process) {
                for (auto i=0U; i<m_count; ++i) {
                    m_regs[i]=quint16(bits[i]);
                }
            }
            if (!exit_signal) {
                emit complete();
            }
        }

        m_mx.unlock();
    } while (!exit_signal);
    modbus_close(m_ctx);
}


void ModbusThread::modbus_request(const quint16 first_reg, const quint16 num_regs, const quint8 uid)
{
    m_mx.lock();
    m_reg_number = first_reg;
    m_count = num_regs;
    m_node = uid;
    m_write_request=false;
    m_raw_request=nullptr;
    m_cond.notify_one();
    m_mx.unlock();
}


std::vector<quint16> ModbusThread::modbus_result()
{
    m_mx.lock();
    std::vector<quint16> result = std::move(m_regs);
    m_regs=std::vector<quint16>();
    m_mx.unlock();

    return result;
}


void ModbusThread::close()
{
    m_mx.lock();
    m_quit=true;
    m_cond.notify_one();
    m_mx.unlock();
    wait();
}


void ModbusThread::modbus_request(const quint16 first_reg, std::vector<quint16> &&regs_to_write, const quint8 uid)
{
    auto reg_count = regs_to_write.size();
    m_mx.lock();
    m_regs = std::move(regs_to_write);
    m_reg_number = first_reg;
    m_count = quint16(reg_count);
    m_node = uid;
    m_write_request=true;
    m_raw_request=nullptr;
    m_cond.notify_one();
    m_mx.unlock();
}


void ModbusThread::modbus_request(const quint8 *pdu, const quint8 length, const qint8 fc, const quint8 uid)
{
    m_mx.lock();
    m_regs = {};
    m_node = uid;
    m_count = quint16(length);
    m_raw_request = pdu;
    m_write_request=false;
    m_reg_number = quint16(fc);
    m_cond.notify_one();
    m_mx.unlock();
}


int ModbusThread::do_write_request()
{
    int result;
    auto write_count=m_regs.size();
    if (1 == write_count && m_reg_number <= 19999) {
        result = modbus_write_bit(m_ctx, int(m_reg_number - 1), (m_regs[0] > 0 ? TRUE : FALSE));
    } else if (m_reg_number <= 19999) {
        std::vector<quint8>write_bits(write_count);
        for (decltype(write_count) i=0; i<write_count; i++) {
            write_bits[i] = (m_regs[i] > 0 ? TRUE : FALSE);
        }
        result = modbus_write_bits(m_ctx, int(m_reg_number - 1), int(write_count), &write_bits[0]);
    } else if (1 == write_count) {
        result = modbus_write_register(m_ctx, int(m_reg_number - 40001), m_regs[0]);
    } else {
        result = modbus_write_registers(m_ctx, int(m_reg_number - 40001), int(write_count), &m_regs[0]);
    }

    return result;
}


quint16 ModbusThread::get_start_reg()
{
    m_mx.lock();
    auto retval = m_reg_number;
    m_mx.unlock();
    return retval;
}


quint8 ModbusThread::get_unit_id()
{
    m_mx.lock();
    auto retval = m_node;
    m_mx.unlock();
    return retval;
}


int ModbusThread::do_custom_request_tcp()
{
    //  Actually looking through the code in libmodbus, their handling of
    // custom functions is hopelessly broken.  Rather than alter the library I
    // thought it best to just put this kludge in.
    std::vector<uint8_t> req(m_count + 2);
    auto fc = uint8_t(m_reg_number);  //  Function code stored in reg_number
    m_reg_number = 0xFFFFU;  //  Always set the "custom function" state.

    req[0] = uint8_t(m_node);
    req[1] = fc;
    for (auto i=0; i<m_count; ++i) {
        req[size_t(i+2)] = uint8_t(m_raw_request[i]);
    }
    auto result = modbus_send_raw_request(m_ctx, req.data(), int(req.size()));
    if (result < 0) {
        return result;
    }

    uint8_t rsp_data[MODBUS_MAX_ADU_LENGTH];
    result = modbus_receive_confirmation(m_ctx, rsp_data);
    if (result < 0) {
        return result;
    }

    auto index=modbus_get_header_length(m_ctx);
    auto fcode = qint16(rsp_data[size_t(index)]);
    if (fcode != fc) {
        errno = int(rsp_data[index + 1]) + MODBUS_ENOBASE;
        return -1;
    }
    auto length = int(rsp_data[4]) * 256;
    length += int(rsp_data[5]) + 6;  //  Add in the Modbus/TCP overhead
    if (length > result) {
        //  Reach in and grab the rest of the packet
        auto sock = modbus_get_socket(m_ctx);
        //auto p = static_cast<void*>(&rsp_data[result]);
        recv(sock, &rsp_data[result], size_t(length - result), MSG_WAITALL);
    }
    index++;
    m_count=0;
    m_regs.resize(size_t(length - index));
    for (; index<length; ++index) {
        m_regs[m_count++] = quint16(rsp_data[size_t(index)]);
    }

    return int(m_count);
}
