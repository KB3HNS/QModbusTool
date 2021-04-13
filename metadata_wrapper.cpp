/**
 * \file metadata_wrapper.cpp
 * \brief Wrapper singleton around the Read Metadata plugin shared object.
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
#include <QStringBuilder>  //  operator %
#include <QCoreApplication>  //  QCoreApplication


// C includes
#include <metadata.h>  //  modbus_plugin: all symbols and definitions in plugin

// project includes
#include "metadata_wrapper.h"  //  local include
#include "exceptions.h"  //  AppException


using namespace std::string_view_literals;


namespace {
    const auto g_dll_name = "mod_plugin.so"sv;
}


MetadataWrapper::MetadataWrapper(const QString &lib_path) :
    QObject(nullptr),
    m_dll_reference{new QLibrary(lib_path, this)}
{
    m_dll_reference->load();
}


MetadataWrapper* MetadataWrapper::get_instance()
{
    static MetadataWrapper *inst = nullptr;
    if (nullptr == inst) {
        const QString dll_path = QCoreApplication::applicationDirPath() %
                                 QChar('/') %
                                 QString(g_dll_name.data());
        inst = new MetadataWrapper(dll_path);
    }

    return inst;
}


bool MetadataWrapper::loaded() const
{
    return m_dll_reference->isLoaded();
}


std::shared_ptr<Metadata> MetadataWrapper::create_request(const quint16 reg_number)
{
    if (!loaded()) {
        throw AppException(tr("Plugin unavailable"));
    }

    quint8 fc;
//    auto p_fn = reinterpret_cast<create>(dlsym(m_dll_reference, CREATE_SYMBOL));
    auto p_fn = reinterpret_cast<create>(m_dll_reference->resolve(CREATE_SYMBOL));
    auto inst = p_fn(reg_number, &fc);
    if (nullptr == inst) {
        throw AppException(tr("Illegal request: ") % QString::number(reg_number));
    }

    return std::make_shared<Metadata>(reg_number, inst, fc);
}


QPair<const quint8*, quint8> MetadataWrapper::encode_request(std::shared_ptr<Metadata> request)
{
//    auto p_fn = reinterpret_cast<encode>(dlsym(m_dll_reference, ENCODE_SYMBOL));
    auto p_fn = reinterpret_cast<encode>(m_dll_reference->resolve(ENCODE_SYMBOL));

    auto length = p_fn(request->m_request_instance, request->m_request.data());
    return {request->m_request.data(), quint8(length)};
}


void MetadataWrapper::decode_response(std::shared_ptr<Metadata> request, const std::vector<quint8> &data)
{
//    auto p_fn = reinterpret_cast<decode>(dlsym(m_dll_reference, DECODE_SYMBOL));
    auto p_fn = reinterpret_cast<decode>(m_dll_reference->resolve(DECODE_SYMBOL));

    if (p_fn(request->m_request_instance, data.data(), uint8_t(data.size())) == 0) {
        decode_labels(request.get());
        decode_defaults(request.get());
        decode_encoding(request.get());
        decode_limits(request.get());
    }
}


void MetadataWrapper::dispose_metadata(const Metadata *const m)
{
//    auto p_fn = reinterpret_cast<release>(dlsym(m_dll_reference, RELEASE_SYMBOL));
    auto p_fn = reinterpret_cast<release>(m_dll_reference->resolve(RELEASE_SYMBOL));
    p_fn(m->m_request_instance);
}


void MetadataWrapper::decode_labels(Metadata *const request)
{
//    auto p_fn = reinterpret_cast<label>(dlsym(m_dll_reference, LABEL_SYMBOL));
    auto p_fn = reinterpret_cast<label>(m_dll_reference->resolve(LABEL_SYMBOL));

    const auto lbl_len = p_fn(request->m_request_instance, nullptr);
    if (lbl_len > 0) {
        auto raw_string = std::vector<char>(size_t(lbl_len));
        p_fn(request->m_request_instance, raw_string.data());
        request->label = QString(raw_string.data());
    }
}


void MetadataWrapper::decode_defaults(Metadata *const request)
{
//    auto p_fn = reinterpret_cast<get_default>(dlsym(m_dll_reference, DEFAULT_SYMBOL));
    auto p_fn = reinterpret_cast<get_default>(m_dll_reference->resolve(DEFAULT_SYMBOL));

    qint32 dflt;
    if (p_fn(request->m_request_instance, &dflt) == 0) {
        request->dflt = dflt;
    }
}


void MetadataWrapper::decode_encoding(Metadata *const request)
{
//    auto p_fn = reinterpret_cast<encoding>(dlsym(m_dll_reference, ENCODING_SYMBOL));
    auto p_fn = reinterpret_cast<encoding>(m_dll_reference->resolve(ENCODING_SYMBOL));

    const auto reported = p_fn(request->m_request_instance);
    if (reported >= 0) {
        request->encoding = RegisterEncoding(reported);
    }
}


void MetadataWrapper::decode_limits(Metadata *const request)
{
//    auto p_fn = reinterpret_cast<minmax>(dlsym(m_dll_reference, MINMAX_SYMBOL));
    auto p_fn = reinterpret_cast<minmax>(m_dll_reference->resolve(MINMAX_SYMBOL));

    qint32 min, max;
    if (p_fn(request->m_request_instance, &min, &max) == 0) {
        request->min = min;
        request->max = max;
    }
}


MetadataWrapper::~MetadataWrapper()
{
    m_dll_reference->unload();
}
