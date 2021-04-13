/**
 * \file scheduler.cpp
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
 */

//  c++ includes
#include <cassert>  //  assert

// C includes
#include <modbus/modbus.h>  //  modbus_strerror

// project includes
#include "scheduler.h"  //  Local include
#include "metadata_wrapper.h"  //  MetadataWrapper


Scheduler::Scheduler(QObject *parent)
    :QObject(parent),
    m_write_requests(),
    m_meta_requests(),
    m_standard_requests(),
    m_modbus_timer{new QTimer(this)}
{
    connect(m_modbus_timer, &QTimer::timeout, this, &Scheduler::modbus_on_timer_expired);
    m_modbus_timer->setSingleShot(true);
}


void Scheduler::start_modbus(ModbusThread *const engine, const std::chrono::milliseconds timeout)
{
    assert(nullptr == m_polling_thread);
    m_polling_thread=engine;
    m_modbus_timer->setInterval(timeout);
    m_write_requests.clear();
    m_meta_requests.clear();
    m_standard_requests.clear();
    m_current_request=nullptr;
    m_active=false;
    connect(engine, &ModbusThread::modbus_error, this, &Scheduler::modbus_on_error);
    connect(engine, &ModbusThread::complete, this, &Scheduler::modbus_on_data);
    m_poll_count = 0;
    m_error_count = 0;
    emit new_register_data(0, SystemRegister::SYSTEM_CONNECTED, 255);
}


void Scheduler::stop_modbus()
{
    if (nullptr != m_polling_thread) {
        disconnect(m_polling_thread, &ModbusThread::modbus_error, this, &Scheduler::modbus_on_error);
        disconnect(m_polling_thread, &ModbusThread::complete, this, &Scheduler::modbus_on_data);
        m_current_request=nullptr;
        m_active=false;
        m_write_requests.clear();
        m_meta_requests.clear();
        m_polling_thread=nullptr;
        if (m_standard_requests.size() > 0) {
            m_standard_requests.clear();
            emit polling_complete();  //  TODO: Really?
        }
        emit new_register_data(0, SystemRegister::SYSTEM_DISCONNECTED, 255);
    }
}


void Scheduler::enqueue_request(BaseDialog *const source)
{
    if (nullptr != m_polling_thread) {
        m_standard_requests.push_back(source);
        figure_next();
    }
}


void Scheduler::modbus_on_write_request(WriteRequest request)
{
    if (nullptr != m_polling_thread) {
        m_write_requests.push_back(request);
        figure_next();
    }
}


void Scheduler::remove_reference(BaseDialog *const screen)
{
    for (auto &i: m_write_requests) {
        if (screen == i.requester) {
            i.requester = nullptr;
        }
    }

    for (auto &i: m_meta_requests) {
        if (screen == i.requester) {
            i.requester = nullptr;
        }
    }

    decltype(m_standard_requests) request_list = {};
    auto start_count = m_standard_requests.size();
    for (const auto i: m_standard_requests) {
        if (i != screen) {
            request_list.push_back(i);
        }
    }
    m_standard_requests = std::move(request_list);

    if (m_current_request == screen) {
        m_current_request = nullptr;
    }

    if (m_standard_requests.size() == 0 && 0 != start_count) {
        emit polling_complete();
    }
}


void Scheduler::modbus_on_error(const int error_code)
{
    m_modbus_timer->stop();
    m_error_count++;
    const QString modbus_error{tr(modbus_strerror(error_code))};
    m_active=false;
    emit poll_exception(m_current_request, modbus_error);

    if (PollAction::POLLING_METADATA == m_current_action) {
        m_meta_requests.pop_front();
    }

    figure_next();
}


void Scheduler::modbus_on_data()
{
    m_modbus_timer->stop();
    m_poll_count++;
    m_active=false;
    if (nullptr == m_polling_thread) {
        //  Escape if no longer connected (IE Queued callback)
    } else if (PollAction::POLLING_METADATA == m_current_action) {
        const auto node = m_polling_thread->get_unit_id();
        emit new_register_data(0, SystemRegister::POLL_METADATA_COMPLETE, node);
        poll_response_metadata();
    } else if (PollAction::POLLING_READ == m_current_action) {
        const auto register_set = m_polling_thread->modbus_result();
        const auto node = m_polling_thread->get_unit_id();
        auto register_number = m_polling_thread->get_start_reg();

        for (const auto i: register_set) {
            emit new_register_data(register_number, i, node);
            ++register_number;
        }
    } else {
        //  Write request
        emit new_register_data(0,
                               SystemRegister::WRITE_REQUEST_COMPLETE,
                               m_polling_thread->get_unit_id());
    }

    figure_next();
}


void Scheduler::modbus_on_timer_expired()
{
    if (m_active) {
        modbus_on_error(11);  //  Device timeout
    }
}


void Scheduler::figure_next()
{
    if (m_active || (nullptr == m_polling_thread)) {
        return;
    }

    m_active = true;
    PollAction next_action = PollAction::POLLING_READ;
    m_current_request = nullptr;
    bool emit_poll_complete = false;
    bool loop;
    do {
        loop = false;
        //  Default: read unless there's nothing to read
        if (m_standard_requests.size() == 0) {
            next_action = PollAction::POLLING_INACTIVE;
        }

        //  Low priority: read metadata
        if (m_meta_requests.size() > 0) {
            next_action = PollAction::POLLING_METADATA;
        }

        //  High priority: write
        if (m_write_requests.size() > 0) {
            next_action = PollAction::POLLING_WRITE;
        }

        switch (next_action) {
        case PollAction::POLLING_WRITE:
            poll_write_request();
            break;

        case PollAction::POLLING_METADATA:
            if (!poll_meta_request()) {
                //  A window is done with polling metadata attempt a read.
                if (m_standard_requests.size() > 0) {
                    next_action = PollAction::POLLING_READ;
                    emit_poll_complete = poll_read_request();
                } else {
                    //  Read queue is empty, scan for something else to do.
                    loop = true;
                }
            }
            break;

        case PollAction::POLLING_READ:
            emit_poll_complete = poll_read_request();
            break;

        case PollAction::POLLING_DEVID:
            poll_devid_request();
            break;

        case PollAction::POLLING_INACTIVE:
//            emit_poll_complete = true;
            m_active = false;
            break;
        }

    } while (loop);

    m_current_action = next_action;

    if (emit_poll_complete) {
        //  This function can't be reentrant.
        emit polling_complete();
    }
}


void Scheduler::poll_write_request()
{
    auto write = m_write_requests.front();
    m_write_requests.pop_front();
    m_current_request = write.requester;
    m_polling_thread->modbus_request(write.first_register, std::move(write.values), write.node);
}


bool Scheduler::poll_meta_request()
{
    auto &cur = m_meta_requests.front();
    auto wrapper = MetadataWrapper::get_instance();
    if (wrapper->loaded() &&
            (cur.current_register <= cur.last_register) &&
            (nullptr != cur.requester)) {
        cur.request = wrapper->create_request(cur.current_register);
        const auto pdu = wrapper->encode_request(cur.request);
        m_polling_thread->modbus_request(pdu.first,
                                         pdu.second,
                                         cur.request->function_code,
                                         cur.node);

        m_current_request = cur.requester;
        return true;
    }

    m_meta_requests.pop_front();
    return false;
}


bool Scheduler::poll_read_request()
{
    m_current_request = m_standard_requests.front();
    m_standard_requests.pop_front();
    m_current_request->poll_register_set(m_polling_thread);
    return (m_standard_requests.size() == 0);
}


void Scheduler::poll_devid_request()
{
    m_current_request = nullptr;
    m_polling_thread->modbus_request(0, 0, 0);
}


void Scheduler::poll_response_metadata()
{
    auto &cur = m_meta_requests.front();
    cur.current_register++;

    const auto register_set = m_polling_thread->modbus_result();
    const auto node = m_polling_thread->get_unit_id();
    std::vector<quint8> rsp(register_set.size());

    for (auto i=register_set.begin(); register_set.end() != i; ++i) {
        rsp[size_t(std::distance(register_set.begin(), i))] = quint8(*i);
    }

    auto wrapper = MetadataWrapper::get_instance();
    wrapper->decode_response(cur.request, rsp);
    if (nullptr != cur.requester) {
        cur.requester->set_metadata(cur.request, node);
    }
}


QPair<quint64, quint64> Scheduler::get_counts() const
{
    return {m_poll_count, m_error_count};
}


bool Scheduler::get_active(BaseDialog* &requester) const
{
    requester = m_current_request;
    return m_active;
}


void Scheduler::modbus_on_poll_meta(WindowMetadataRequest request_sequence)
{
    if (nullptr != m_polling_thread) {
        m_meta_requests.push_back(request_sequence);
        figure_next();
    }
}
