/**
 * \file register_display.cpp
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
 */


//  c++ includes
#include <chrono>  //  std::chrono::seconds
#include <QStringList>  //  QStringList
#include <QFileDialog>  //  QFileDialog
#include <QMessageBox>  //  QMessageBox
#include <QScrollBar>  //  QScrollBar
#include <QSpacerItem>  //  QSpacerItem
#include <qtcsv/stringdata.h>  //  QtCSV::StringData
#include <qtcsv/writer.h>  //  QtCSV::Writer::write

// C includes
/* -none- */

// project includes
#include "register_display.h"  //  local include
#include "metadata_wrapper.h"  //  MetadataWrapper
#include "scheduler.h"  //  SystemRegister


RegisterDisplay::RegisterDisplay(QWidget *parent, const quint16 base_reg, const quint16 count, const quint8 uid)
        : BaseDialog(parent),
          m_starting_register{base_reg},
          m_count{count},
          m_node{uid},
          m_register_labels(),
          m_register_values(),
          m_register_descriptions(),
          m_register_encoding(count, RegisterEncoding::ENCODING_NONE),
          m_raw_values(count, 0),
          m_scroll_area{new QScrollArea(this)},
          m_scroll_container{new QWidget(m_scroll_area)},
          m_scroll_layout{new QGridLayout(m_scroll_container)},
          m_status{new QStatusBar(this)},
          m_control_box{new QGroupBox(tr("Settings"), m_scroll_container)},
          m_control_grid{new QGridLayout(m_control_box)},
          m_reg_select{new QSpinBox(m_control_box)},
          m_node_select{new QSpinBox(m_control_box)},
          m_quantity{new QSpinBox(m_control_box)},
          m_apply_button{new QPushButton(tr("Apply\nChanges"), m_control_box)},
          m_refresh_button{nullptr},
          m_save_button{new QPushButton(tr("Save\nData"), m_control_box)},
          m_status_timer{new QTimer(this)}
{
    if (base_reg <= 19999) {
        m_max_regs=0x07D0;
    } else {
        m_max_regs=125;
    }

    m_status_timer->setSingleShot(true);
    m_status_timer->setInterval(std::chrono::seconds(30));
    connect(this, &RegisterDisplay::window_closed, this, [=](QWidget*) {
        deleteLater();
    });
}


void RegisterDisplay::setupUi()
{
    for (quint16 i=0U; i<m_count; i++) {
        auto reg = new QLabel(m_scroll_container);
        auto reg_value = create_value_widget(i, true);
        auto reg_descr = new QLabel(m_scroll_container);
        reg->setText(get_register_number_text(m_starting_register + i));
        m_scroll_layout->addWidget(reg, i+1, 0);
        m_scroll_layout->addWidget(reg_value, i+1, 1);
        m_scroll_layout->addWidget(reg_descr, i+1, 2);
        m_register_labels.push_back(reg);
        m_register_values.push_back(reg_value);
        m_register_descriptions.push_back(reg_descr);
    }

    m_top_layout->addWidget(m_scroll_area);
    m_top_layout->addWidget(m_status);
    m_top_layout->setSizeConstraint(QLayout::SetMinimumSize);
    m_top_layout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scroll_area->setWidget(m_scroll_container);
    m_scroll_container->setLayout(m_scroll_layout);
    m_scroll_layout->setVerticalSpacing(15);
    m_scroll_area->setFrameShape(QFrame::NoFrame);
    m_scroll_area->setFrameRect(QRect(0, 0, 0, 0));
    m_scroll_area->setWidgetResizable(true);
    
    /* Now, set-up the control area */
    m_scroll_layout->addWidget(m_control_box, 0, 0, 1, 3);
    m_control_box->setLayout(m_control_grid);
    m_control_grid->setHorizontalSpacing(20);
    m_control_grid->addItem(new QSpacerItem(0, 15), 0, 0, 0, 3);
    m_control_grid->addWidget(new QLabel(tr("Start Register"), m_control_box), 1, 0);
    m_control_grid->addWidget(m_reg_select, 2, 0);
    if (m_starting_register <= 9999) {
        m_reg_select->setRange(1, 9999);
    } else if (m_starting_register <= 19999) {
        m_reg_select->setRange(10001, 19999);
    } else if (m_starting_register <= 39999) {
        m_reg_select->setRange(30001, 39999);
    } else {
        m_reg_select->setRange(40001, 49999);
    }
    m_reg_select->setValue(int(m_starting_register));

    m_control_grid->addWidget(new QLabel(tr("Register Count"), m_control_box), 1, 1);
    m_control_grid->addWidget(m_quantity, 2, 1);
    m_quantity->setRange(1, int(m_max_regs));
    m_quantity->setValue(int(m_count));

    m_control_grid->addWidget(new QLabel(tr("Node/Unit ID Select"), m_control_box), 3, 0);
    m_control_grid->addWidget(m_node_select, 4, 0);
    m_node_select->setRange(0, 255);
    m_node_select->setValue(int(m_node));

    m_control_grid->addWidget(m_apply_button, 1, 2, 2, 1);
    add_icon_to_button(m_apply_button, QStyle::SP_DialogApplyButton);
    connect(m_apply_button, &QPushButton::clicked, this, &RegisterDisplay::on_apply_clicked);
    m_apply_button->setDefault(true);
    const auto wrapper = MetadataWrapper::get_instance();
    if (wrapper->loaded()) {
        m_refresh_button = new QPushButton(tr("Requery\nMetadata"), m_control_box);
        add_icon_to_button(m_refresh_button, QStyle::SP_BrowserReload);
        m_control_grid->addWidget(m_refresh_button, 3, 1, 2, 1);
        connect(m_refresh_button, &QPushButton::clicked, this, &RegisterDisplay::on_refresh_clicked);
    }
    m_control_grid->addWidget(m_save_button, 3, 2, 2, 1);
    add_icon_to_button(m_save_button, QStyle::SP_DialogSaveButton);
    connect(m_save_button, &QPushButton::clicked, this, &RegisterDisplay::on_save_clicked);

    m_scroll_area->show();
    m_control_box->show();
    m_status->show();
    set_title();

    resize(450, 485);
//    m_debug_resize = true;
}


void RegisterDisplay::poll_register_set(ModbusThread *const engine)
{
    engine->modbus_request(m_starting_register, m_count, quint8(m_node_select->value()));
}


void RegisterDisplay::on_new_value(const quint16 reg, const quint16 value, const quint8 unit_id)
{
    if (0 == reg) {
        //  System register
        if (SystemRegister::SYSTEM_CONNECTED == value ||
                SystemRegister::SYSTEM_DISCONNECTED == value) {
            m_meta_in_process = false;
        }
    } else if (unit_id == m_node && reg >= m_starting_register) {
        const auto reg_idx = size_t(reg - m_starting_register);
        if (reg_idx < m_count) {
            m_raw_values[reg_idx] = value;
            auto decoded_value = decode_register(value, m_register_encoding[reg_idx]);
            updateRegisterValue(reg_idx, decoded_value);
        }
    } else {

    }
}


void RegisterDisplay::updateRegisterValue(const size_t index, const QString &value)
{
    auto *lbl = dynamic_cast<QLabel*>(m_register_values[index]);
    lbl->setText(value);
}


void RegisterDisplay::on_apply_clicked()
{
    m_have_metadata = false;
    quint16 new_count = quint16(m_quantity->value());
    m_register_encoding.resize(size_t(m_quantity->value()));
    m_raw_values.resize(size_t(m_quantity->value()));
    while (new_count < m_count) {
        auto label = m_register_labels.back();
        m_register_labels.pop_back();
        m_scroll_layout->removeWidget(label);
        label->deleteLater();

        auto reg = m_register_values.back();
        m_scroll_layout->removeWidget(reg);
        m_register_values.pop_back();
        reg->deleteLater();

        auto descr = m_register_descriptions.back();
        m_scroll_layout->removeWidget(descr);
        m_register_descriptions.pop_back();
        descr->deleteLater();

        m_count--;
    }

    while (new_count > m_count) {
        m_count++;

        auto label = new QLabel(m_scroll_area);
        m_scroll_layout->addWidget(label, int(m_count), 0);
        m_register_labels.push_back(label);
        label->show();

        auto reg = create_value_widget(m_count, false);
        m_scroll_layout->addWidget(reg, int(m_count), 1);
        m_register_values.push_back(reg);
        reg->show();

        auto descr = new QLabel(m_scroll_area);
        m_scroll_layout->addWidget(descr, int(m_count), 2);
        m_register_descriptions.push_back(descr);
        descr->show();
    }

    m_starting_register=quint16(m_reg_select->value());
    for (quint16 i=0; i<new_count; i++) {
        m_register_labels[i]->setText(get_register_number_text(i + m_starting_register));
        m_register_labels[i]->setText(QString::number(int(i + m_starting_register)));
        m_register_encoding[i]=RegisterEncoding::ENCODING_NONE;
        m_register_descriptions[i]->setText("");
    }

    m_meta_in_process = false;
    set_title();
}


void RegisterDisplay::on_refresh_clicked()
{
    m_have_metadata = false;
    if (!m_meta_in_process) {
        m_meta_in_process = true;
        auto meta_request = WindowMetadataRequest{};
        meta_request.requester = this;
        meta_request.current_register = m_starting_register;
        meta_request.last_register = m_starting_register + m_count - 1;
        meta_request.node = m_node;
        emit metadata_requested(meta_request);
    }
}


QString RegisterDisplay::decode_register(const quint16 value, const RegisterEncoding encoding) const
{
    QString decoded_value;
    switch (encoding) {
    case RegisterEncoding::ENCODING_NONE:
    case RegisterEncoding::ENCODING_UINT16:
    case RegisterEncoding::ENCODING_UNKNOWN:
    case RegisterEncoding::ENCODING_USER:
        decoded_value=QString::number(int(value));
        break;

    case RegisterEncoding::ENCODING_BITS:
        decoded_value = QString("0x") % QString::number(int(value), 16);
        break;

    case RegisterEncoding::ENCODING_SIGNED_BYTES: {
            const auto ia = static_cast<qint8>(quint8(value >> 8));
            const auto ib = static_cast<qint8>(quint8(value));
            decoded_value = QString::number(int(ia)) % QChar(',') % QString::number(int(ib));
        } break;

    case RegisterEncoding::ENCODING_BYTES: {
            const auto ua = quint8(value >> 8);
            const auto ub = quint8(value);
            decoded_value = QString::number(int(ua))  % QChar(',') % QString::number(int(ub));
        } break;

    case RegisterEncoding::ENCODINT_INT16: {
            const auto v = static_cast<qint16>(value);
            decoded_value = QString::number(int(v));
        } break;
    }

    return decoded_value;
}


void RegisterDisplay::on_exception_status(BaseDialog *const requester, const QString exception)
{
    if (this == requester) {
        m_status->showMessage(exception);
        m_status_timer->start();
        m_meta_in_process = false;
    }
}


QWidget* RegisterDisplay::create_value_widget(const quint16 index, const bool initial)
{
    static_cast<void>(index);
    static_cast<void>(initial);
    return new QLabel(m_scroll_area);
}


void RegisterDisplay::on_status_timer_timeout()
{
    m_status->showMessage("");
}


QString RegisterDisplay::get_object_name() const
{
    return "RegisterDisplay";
}


void RegisterDisplay::save_configuration_parameters(QDomElement &node) const
{
    node.setAttribute("register", QString::number(m_starting_register));
    node.setAttribute("count", QString::number(m_count));
    node.setAttribute("node", QString::number(m_node));
    node.setAttribute("max", QString::number(m_max_regs));

    const auto position = pos();
    const auto w_size = size();
    node.setAttribute("w", QString::number(w_size.width()));
    node.setAttribute("h", QString::number(w_size.height()));
    node.setAttribute("x", QString::number(position.x()));
    node.setAttribute("y", QString::number(position.y()));

    const QScrollBar *adj = m_scroll_area->verticalScrollBar();
    node.setAttribute("scroll", QString::number(adj->value()));
}


bool RegisterDisplay::load_configuration_parameters(const QDomElement &node)
{
    auto max = node.attribute("max").toUInt();
    if (max == m_max_regs) {
        const auto x = node.attribute("x", "");
        const auto y = node.attribute("y", "");
        if (x.length() > 0 && y.length() > 0) {
            move(x.toInt(), y.toInt());
        }

        const auto h = node.attribute("h", "-1").toInt();
        const auto w = node.attribute("w", "-1").toInt();
        const auto scroll = node.attribute("scroll", "-1").toInt();
        if (h > 0 && w > 0 && scroll >= 0) {
            connect(this, &RegisterDisplay::window_first_display, this, [=](BaseDialog *window) {
                static_cast<void>(window);
                QScrollBar *adj = m_scroll_area->verticalScrollBar();
                adj->setValue(scroll);
                resize(w, h);
            });
        }
        return true;
    }

    return false;
}


void RegisterDisplay::set_metadata(std::shared_ptr<Metadata> metadata, const quint8 node)
{
    if (node == m_node && metadata->register_number >= m_starting_register &&
            metadata->register_number < m_starting_register + m_count) {
        const size_t index = metadata->register_number - m_starting_register;
        m_register_descriptions[index]->setText(metadata->label);
        m_register_encoding[index] = metadata->encoding;
        if (index == (m_count - 1)) {
            m_meta_in_process = false;
            m_have_metadata = true;
        }
    } else {
        m_meta_in_process = false;
    }
}


void RegisterDisplay::on_save_clicked()
{
    auto dialog = QFileDialog(this, tr("Save data as..."));
    dialog.setNameFilters({tr("Spreadsheet (*.csv)"),
                           tr("All files (*)")});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("csv");
    if (dialog.exec() != 0) {
        auto file_name = dialog.selectedFiles().first();
        if (!save_register_set(file_name)) {
            QMessageBox::warning(this, tr("Save registers"),
                                 tr("Cannot write file %1:\n.")
                                 .arg(QDir::toNativeSeparators(file_name)));
        }
    }
}


QString RegisterDisplay::get_display_value(const size_t index) const
{
    const auto *lbl = dynamic_cast<QLabel*>(m_register_values[index]);
    return lbl->text();
}


bool RegisterDisplay::save_register_set(const QString &path)
{
    const std::vector<QString> encodings = {
        tr("None"),
        tr("Unsigned-16"),
        tr("Signed-16"),
        tr("Signed Bytes"),
        tr("Unsigned Bytes"),
        tr("Bits"),
        tr("User-Defined"),
        tr("Unknown")
    };

    QStringList header;
    header << tr("Register number");
    if (m_have_metadata) {
        header << tr("Label");
    }
    header << tr("Value") << tr("Device ID/Node");
    if (m_have_metadata) {
        header << tr("Raw Value") << tr("Encoding");
    }

    QtCSV::StringData csv_data;
    csv_data.addRow(header);

    for (quint16 i=0U; i<m_count; ++i) {
        QStringList row;
        row << QString::number(m_starting_register + i);
        if (m_have_metadata) {
            row << m_register_descriptions[i]->text();
        }
        row << get_display_value(i)
            << QString::number(m_node);
        if (m_have_metadata) {
            row << QString::number(m_raw_values[i])
                << encodings[size_t(m_register_encoding[i])];
        }

        csv_data.addRow(row);
    }

    return QtCSV::Writer::write(path, csv_data);
}


QString RegisterDisplay::get_register_number_text(const quint16 reg_number)
{
    return QString::number(reg_number);
}


void RegisterDisplay::set_title()
{
    setWindowTitle(get_register_number_text(m_starting_register) %
                   " - " %
                   get_register_number_text(m_starting_register + m_count - 1) %
                   '@' %
                   QString::number(m_node));
}
