/**
 * \file base_dialog.cpp
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
 */


//  c++ includes
#include <QDebug>  //  qDebug
#include <QIcon>  //  QIcon

// C includes
/* -none- */

// project includes
#include "base_dialog.h"  //  local include
#include "exceptions.h"


BaseDialog::BaseDialog(QWidget *const parent, const bool create_layout) :
    QDialog(parent),
    m_top_layout{(create_layout ? new QVBoxLayout(this) : nullptr)},
    m_first_display{true},
    m_emit_close{true}
{
    if (create_layout) {
        setLayout(m_top_layout);
    }

    connect(this, &BaseDialog::rejected, this, [=]() {
        if (m_emit_close) {
            m_emit_close = false;
            emit window_closed(this);
        }
    });

    //  Solution found at:
    // https://stackoverflow.com/questions/42655988/adjust-title-position-in-a-qgroupbox-using-style-sheets
    // To the Qt developers:  Why the   ~ [ H 3 1 ! ] ~   is this not the
    // default and where the   ~ [ F * * * ] ~   is this documented?
    //  I mean, come on!  Scroll area has a frame by default which no one uses,
    // but this requires this long of a string to be consistent with every
    // other WM out there?  Really?
    setStyleSheet("QGroupBox{"
                      "font: bold;"
                      "border: 1px solid silver;"
                      "border-radius: 6px;"
                      "margin-top: 6px;} "
                  "QGroupBox::title{"
                      "subcontrol-origin: margin;"
                      "left: 7px;"
                      "top: -5px;"
                      "padding: 0px 5px 0px 5px}");

    auto icon = QIcon(":/QModbusTool.ico");
    setWindowIcon(icon);
}


void BaseDialog::showEvent(QShowEvent *evt)
{
    if (m_first_display) {
        m_first_display = false;
        setupUi();
        emit window_first_display(this);
    }
    QDialog::showEvent(evt);
}


void BaseDialog::resizeEvent(QResizeEvent *evt)
{
    QDialog::resizeEvent(evt);
    if (m_debug_resize) {
        const auto size = evt->size();
        qDebug() << size.width() << ',' << size.height();
    }
}


void BaseDialog::closeEvent(QCloseEvent *evt)
{
    QDialog::closeEvent(evt);
    if (m_emit_close) {
        m_emit_close = false;
        emit window_closed(this);
    }
}


void BaseDialog::add_icon_to_button(QAbstractButton *const button,
                                    const QStyle::StandardPixmap icon_name) const
{
    //  As far as I'm concerned, the fact that I need to do this is a bug in QT.
    // https://forum.learnpyqt.com/t/are-there-any-built-in-qicons/185/2
    auto style = button->style();
    auto icon = style->standardIcon(icon_name);
    button->setIcon(icon);
}


void BaseDialog::on_new_value(const quint16 reg, const quint16 value, const quint8 unit_id)
{
    static_cast<void>(reg);
    static_cast<void>(value);
    static_cast<void>(unit_id);
}


void BaseDialog::on_exception_status(BaseDialog *requester, const QString exception)
{
    static_cast<void>(requester);
    static_cast<void>(exception);
}


void BaseDialog::set_metadata(std::shared_ptr<Metadata> metadata, const quint8 node)
{
    static_cast<void>(metadata);
    static_cast<void>(node);
    throw AppException("Metadata not configured in this object");
}


void BaseDialog::poll_register_set(ModbusThread *const engine)
{
    static_cast<void>(engine);
    throw AppException("Polling not configured in this object");
}
