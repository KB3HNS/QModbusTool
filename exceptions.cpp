/**
 * \file exceptions.cpp
 * \brief Exceptions used throughout the application
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

// C includes
/* -none- */

// project includes
#include "exceptions.h"  //  local include


AppException::AppException(const QString &reason) :
    QException(),
    text(reason)
{
}


AppException::operator QString () const
{
    return text;
}


AppException* AppException::clone() const
{
    return new AppException(*this);
}


void AppException::raise() const
{
    throw *this;
}


const char* AppException::what() const noexcept
{
    return text.toLocal8Bit();
}


FileLoadException::FileLoadException(const QString &reason, const QString &filename) :
    AppException(reason % QString(": ") % filename)
{
}


FileLoadException::FileLoadException(const QString &reason, const QString &filename, const size_t line) :
    AppException(reason %
                 QString(": ") %
                 filename %
                 QChar('@') %
                 QString::number(line))
{
}
