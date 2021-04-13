/**
 * \file metadata_wrapper.h
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
 *
 * \section DESCRIPTION
 *
 * The MetadataWrapper class is a singleton class providing access the
 * metadata modbus plugin (non-GPL).  It also provides safe abstraction so
 * functionality isn't lost (aside from the data its self) if the plugin isn't
 * present.  It also allows for easy replacement with another interface that
 * can provide the same information.
 */

#ifndef METADATA_WRAPPER_H
#define METADATA_WRAPPER_H


//  c++ includes
#include <QtCore>  // QObject
#include <QPair>  //  QPair
#include <QLibrary>  //  QLibrary
#include <string_view>  //  std::string_view
#include <vector>  //  std::vector
#include <memory>  //  std::shared_ptr
#include <string>  //  std::string

// C includes
/* -none- */

// project includes
#include "metadata_structs.h"  //  Metadata


/**
 * \brief State-aware wrapper around the metadata modbus plugin
 */
class MetadataWrapper : protected QObject
{
    Q_OBJECT
//    friend class Metadata;

public:

    /**
     * \brief Test if the library has been successfully loaded
     * @return ``true`` if loaded, ``false`` otherwise
     */
    [[nodiscard]] bool loaded() const;

    /**
     * \brief Generate request container
     * @param reg_number Request metadata for this register number
     * @return Metadata container
     */
    [[nodiscard]] std::shared_ptr<Metadata> create_request(const quint16 reg_number);

    /**
     * \brief Generate an outgoing request PDU
     * @param request Request container
     * @return raw PDU (length and data pointer)
     */
    [[nodiscard]] QPair<const quint8*, quint8> encode_request(std::shared_ptr<Metadata> request);

    /**
     * \brief Decode a response PDU
     * @param request Request container (updated)
     * @param data response PDU
     */
    void decode_response(std::shared_ptr<Metadata> request, const std::vector<quint8> &data);

    /**
     * \brief This class is a singleton, get the instance.
     * @return Singleton instance
     */
    static MetadataWrapper* get_instance();

    /**
     * \brief Dispose metadata during destructor
     * \note
     * This function is only called by the MetaData object, and only during
     * destruction.  It is responsible for instructing the plugin to free
     * internal resources.
     *
     * @param m pointer to metadata object
     */
    void dispose_metadata(const Metadata *const m);

    virtual ~MetadataWrapper();

private:

    /**
     * \brief constructor
     * @param lib_path Path to the shared object
     */
    MetadataWrapper(const QString &lib_path);

    /**
     * \fn decode_labels, decode_defaults, decode_encoding, decode_limits
     * \brief Internal decoder functions
     * @param request pointer to request instance
     */
    void decode_labels(Metadata *const request);
    void decode_defaults(Metadata *const request);
    void decode_encoding(Metadata *const request);
    void decode_limits(Metadata *const request);

    QLibrary *const m_dll_reference;
//    void *m_dll_reference = nullptr;
};

#endif // METADATA_WRAPPER_H
