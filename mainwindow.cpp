/**
 * \file mainwindow.cpp
 * \brief Main window for program
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
#include <chrono>  //  std::chrono::milliseconds
#include <vector>  //  std::vector
#include <string_view>  //  std::swap (as of c++17)
#include <QFileDialog>  //  QFileDialog
#include <QMessageBox>  //  QMessageBox
#include <QTextStream>  //  QTextStream
#include <qtcsv/reader.h>  //  QtCSV::Reader::readToList

// C includes
#include <modbus/modbus.h>  //  modbus_strerror

// project includes
#include "mainwindow.h"  //  local include
#include "holding_register_display.h"  //  HoldingRegisterDisplay
#include "coils_display.h"  //  CoilsDisplay
#include "inputs_display.h"  //  InputsDisplay
#include "exceptions.h"  //  AppException, FileLoadException
#include "csv_importer.h"  //  CsvImporter
#include "metadata_wrapper.h"  //  MetadataWrapper


using BaseData = std::tuple<quint8, quint16, quint16>;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::MainWindow), m_connected(false),
      m_register_windows(),
      m_scheduler{new Scheduler(this)},
      m_update_timer{new QTimer(this)},
      m_trend{nullptr}
{
    m_ui->setupUi(this);
    m_update_timer->setInterval(std::chrono::milliseconds(100));
    m_update_timer->setSingleShot(false);
    connect(m_update_timer, &QTimer::timeout, this, &MainWindow::update_timer_on_expired);
    connect(m_scheduler, &Scheduler::poll_exception, this, &MainWindow::modbus_on_error);
    connect(m_scheduler, &Scheduler::polling_complete, this, &MainWindow::polling_on_complete);
    const auto wrapper = MetadataWrapper::get_instance();
    if (!wrapper->loaded()) {
        m_ui->actionRead_Metadata->setEnabled(false);
        m_ui->actionRead_Metadata->setToolTip(tr("Plugin unavailable"));
    }
}


MainWindow::~MainWindow()
{
    delete m_ui;
}


void MainWindow::on_actionConnect_triggered()
{
    m_ui->actionContinuous->setChecked(false);
    if (m_connected) {
        m_ui->statusbar->showMessage("");
        m_engine->close();
        post_disconnected();
    } else {
        m_connecting=true;
        m_connecting2=false;
        m_ui->statusbar->showMessage(tr("..."));
        m_ui->actionConnect->setEnabled(false);
        m_ui->ipEdit->setEnabled(false);
        m_ui->portEdit->setEnabled(false);
        m_ui->timeoutEdit->setEnabled(false);
        m_engine = new ModbusThread(this, m_ui->ipEdit->text(), quint16(m_ui->portEdit->text().toInt()));
        connect(m_engine, &ModbusThread::complete, this, &MainWindow::modbus_on_data);\
        connect(m_engine, &ModbusThread::modbus_error, this, &MainWindow::modbus_on_error_protocol);
        //  TODO: Raw error / route startup through the scheduler.
        m_engine->start();
    }
}


void MainWindow::on_actionCoils_triggered()
{
    add_window(new CoilsDisplay(this, 1));
}


void MainWindow::on_actionInputs_triggered()
{
    add_window(new InputsDisplay(this, 10001));
}


void MainWindow::on_actionInput_Registers_triggered()
{
    add_window(new RegisterDisplay(this, 30001));
}


void MainWindow::on_actionHolding_Registers_triggered()
{
    add_window(new HoldingRegisterDisplay(this, 40001));
}


void MainWindow::closeEvent(QCloseEvent *evt)
{
    if (m_connected) {
        m_engine->close();
        post_disconnected();
    }
    const std::vector<RegisterDisplay*> windows_to_close(m_register_windows.begin(),
                                                         m_register_windows.end());
    m_register_windows.clear();
    for (auto i: windows_to_close) {
        i->close();
        i->deleteLater();
    }

    if (nullptr != m_trend) {
        m_trend->close();
    }

    QMainWindow::closeEvent(evt);
}


void MainWindow::modbus_on_error(BaseDialog *requester, const QString exception)
{
    if (nullptr == requester) {
        m_ui->statusbar->showMessage(exception);
    }
}


void MainWindow::modbus_on_data()
{
    if (m_connecting) {
        //post_connected("");
        m_connecting = false;
        m_connecting2 = true;
        m_engine->modbus_request(0, 0, 0);
        m_ui->statusbar->showMessage(tr("Connected to %1:%2")
                                     .arg(m_ui->ipEdit->text())
                                     .arg(m_ui->portEdit->text()));
        post_connected();
    } else if (m_connecting2) {
        auto data = m_engine->modbus_result();
        //  strip off null terminator and RUN/STOP indicator
        QString device_id(int(data.size()) - 2, ' ');
        for (int i=0; i < device_id.size(); ++i) {
            device_id[i] = QChar(uchar(data[unsigned(i)]));
        }
        m_ui->statusbar->showMessage(tr("Connected to %1").arg(device_id));
        m_connecting2=false;
    } else {
    }
}


void MainWindow::post_connected()
{
    m_active = false;
    m_update_timer->start();

    auto timeout = m_ui->timeoutEdit->text().toInt();
    if (timeout < 1) {
        m_ui->timeoutEdit->setText("3000");
        timeout=3000;
    }
    m_scheduler->start_modbus(m_engine, std::chrono::milliseconds(timeout));
    m_ui->actionConnect->setEnabled(true);

    m_ui->menuPoll->setEnabled(true);
    m_ui->actionConnect->setText(tr("Disconnect"));
    m_connected = true;
}


void MainWindow::post_disconnected()
{
    m_scheduler->stop_modbus();
    m_connecting=false;
    m_connecting2=false;
    m_ui->actionConnect->setEnabled(true);
    m_ui->ipEdit->setEnabled(true);
    m_ui->portEdit->setEnabled(true);
    m_ui->timeoutEdit->setEnabled(true);
    m_ui->actionConnect->setText(tr("Connect"));
    m_ui->actionContinuous->setChecked(false);
    m_ui->menuPoll->setEnabled(false);
    m_update_timer->stop();
    m_active = false;
    disconnect(m_engine, &ModbusThread::modbus_error, this, &MainWindow::modbus_on_error_protocol);
    disconnect(m_engine, &ModbusThread::complete, this, &MainWindow::modbus_on_data);
    m_engine=nullptr;
    m_connected = false;
}


void MainWindow::on_actionOnce_triggered()
{
    //  Note:  I could put a re-entrancy guard here, but I don't think that
    // the behavior is really a problem.
    if (m_connected) {
        if (m_register_windows.size() == 0) {
            m_ui->actionContinuous->setChecked(false);
        } else {
            for (auto i: m_register_windows) {
                m_scheduler->enqueue_request(i);
            }
        }
    }
}


void MainWindow::on_actionContinuous_triggered()
{
    if (m_connected && m_ui->actionContinuous->isChecked()) {
        on_actionOnce_triggered();
    }
}


void MainWindow::register_window_destroyed(BaseDialog *window)
{
    const auto element = m_register_windows.find(dynamic_cast<RegisterDisplay*>(window));
    if (m_register_windows.end() != element) {
        disconnect(m_scheduler, &Scheduler::new_register_data,
                   *element, &RegisterDisplay::on_new_value);
        disconnect(m_scheduler, &Scheduler::poll_exception,
                   *element, &RegisterDisplay::on_exception_status);
        disconnect(*element, &RegisterDisplay::write_requested,
                   m_scheduler, &Scheduler::modbus_on_write_request);
        disconnect(*element, &RegisterDisplay::metadata_requested,
                   m_scheduler, &Scheduler::modbus_on_poll_meta);
        m_register_windows.erase(element);
        m_scheduler->remove_reference(window);
    }
}


void MainWindow::add_window(RegisterDisplay *window)
{
    window->show();
    m_register_windows.insert(window);
    connect(m_scheduler, &Scheduler::new_register_data, window, &RegisterDisplay::on_new_value);
    connect(m_scheduler, &Scheduler::poll_exception, window, &RegisterDisplay::on_exception_status);
    connect(window, &RegisterDisplay::write_requested, m_scheduler, &Scheduler::modbus_on_write_request);
    connect(window, &RegisterDisplay::metadata_requested, m_scheduler, &Scheduler::modbus_on_poll_meta);
    connect(window, &RegisterDisplay::window_closed, this, &MainWindow::register_window_destroyed);
}


void MainWindow::update_timer_on_expired()
{
    BaseDialog *unused;
    if (m_scheduler->get_active(unused)) {
        const auto counts = m_scheduler->get_counts();
        m_ui->statusbar->showMessage(tr("Polling: (rx: ") %
                                     QString::number(counts.first) %
                                     tr(" / err: ") %
                                     QString::number(counts.second) %
                                     QChar(')'));
        m_active = true;
    } else if (m_active) {
        m_ui->statusbar->showMessage(tr("Idle"));
    } else {

    }
}


void MainWindow::polling_on_complete()
{
    if (m_connected && m_ui->actionContinuous->isChecked()) {
        on_actionOnce_triggered();
    }
}


void MainWindow::on_actionSave_Session_triggered()
{
    auto dialog = QFileDialog(this, tr("Save session as..."));
    dialog.setNameFilters({tr("Session (*.qmbs)"),
                           tr("All files (*)")});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec() != 0) {
        auto file_name = dialog.selectedFiles().first();
        auto f = QFile(file_name);
        if (f.open(QFile::WriteOnly | QFile::Text)) {
            auto config = save_config();
            auto out = QTextStream(&f);
            out << "<?xml version=\"1.0\"?>\n";
            config.save(out, 0);
            f.close();
        } else {
            QMessageBox::warning(this, tr("Save Session"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(file_name))
                                 .arg(f.errorString()));
        }
    }
}


void MainWindow::on_actionRestore_Session_triggered()
{
    if (m_connected) {
        auto message_box = QMessageBox(this);
        message_box.setText(tr("Close running session"));
        message_box.setInformativeText(tr("Disconnect currently running host?"));
        message_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        message_box.setDefaultButton(QMessageBox::Cancel);
        if (message_box.exec() != QMessageBox::Ok) {
            return;
        }
        on_actionConnect_triggered();
    }

    auto dialog = QFileDialog(this, tr("Load session..."));
    dialog.setNameFilters({tr("Session (*.qmbs)"),
                           tr("All files (*)")});
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec() != 0) {
        const auto filename = dialog.selectedFiles().first();
        auto ip_text = m_ui->ipEdit->text();
        auto port_text = m_ui->portEdit->text();
        auto timeout_text = m_ui->timeoutEdit->text();
        auto old_windows = std::unordered_set<RegisterDisplay*>(
                    m_register_windows.begin(), m_register_windows.end());
        auto old_trend = m_trend;
        m_trend = nullptr;

        auto success = true;
        try {
            load_config(filename);
        } catch (const FileLoadException &e) {
            QMessageBox::warning(this, tr("Load error"),
                                 tr("Error loading: %1\nError reported was %2")
                                 .arg(QDir::toNativeSeparators(filename))
                                 .arg(QString(e)));

            success = false;
            if (nullptr != m_trend) {
                auto temp = m_trend;
                trend_on_closed(temp);
                temp->close();
                emit temp->deleteLater();
                m_trend = old_trend;
            }
            for (auto i: m_register_windows) {
                if (old_windows.find(i) != old_windows.end()) {
                    i->close();
                }
            }

            m_ui->ipEdit->setText(ip_text);
            m_ui->portEdit->setText(port_text);
            m_ui->timeoutEdit->setText(timeout_text);
        } if (success) {
            if (nullptr != old_trend) {
                /* swap old and new so we can close the old */
                std::swap(m_trend, old_trend);

                auto temp = m_trend;
                trend_on_closed(temp);
                temp->close();
                emit temp->deleteLater();

                /* Restore new pointer */
                m_trend = old_trend;
            }
            for (auto i: old_windows) {
                i->close();
            }
        }
    }

    m_ui->actionTrend->setEnabled((nullptr == m_trend));
}


QDomDocument MainWindow::save_config() const
{
    auto document = QDomDocument();
    auto root = document.createElement("QtModbusTool_Session");
    root.setAttribute("version", "1.0");
    root.setAttribute("revision", "1.0");
    document.appendChild(root);
    auto core = document.createElement("communications");
    auto windows = document.createElement("windows");
    root.appendChild(core);
    root.appendChild(windows);

    auto common = document.createElement("common");
    common.setAttribute("method", "TCP");
    common.setAttribute("timeout", m_ui->timeoutEdit->text());
    core.appendChild(common);

    const auto position = pos();
    const auto w_size = size();
    core.setAttribute("w", QString::number(w_size.width()));
    core.setAttribute("h", QString::number(w_size.height()));
    core.setAttribute("x", QString::number(position.x()));
    core.setAttribute("y", QString::number(position.y()));

    auto tcp = document.createElement("TCP");
    tcp.setAttribute("ip", m_ui->ipEdit->text());
    tcp.setAttribute("port", m_ui->portEdit->text());
    core.appendChild(tcp);

    for (auto i: m_register_windows) {
        auto window = document.createElement(i->get_object_name());
        windows.appendChild(window);
        i->save_configuration_parameters(window);
    }

    if (nullptr != m_trend) {
        const auto trend = m_trend->save_configuration(document);
        root.appendChild(trend);
    }

    return document;
}


void MainWindow::load_config(const QString &filename)
{
    auto f = QFile(filename);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        throw FileLoadException(tr("Unable to open file"), filename);
    }

    auto document = QDomDocument();
    auto loaded = document.setContent(&f);
    f.close();
    if (!loaded) {
        throw FileLoadException(tr("Unable to parse file"), filename);
    }

    auto root = document.documentElement();
    if (root.nodeName() != "QtModbusTool_Session" ||
            root.attribute("version").toFloat() != 1.0f ||
            root.attribute("revision").toFloat() < 1.0f) {
        throw FileLoadException(tr("Invalid file"), filename);
    }

    const auto &comms = root.firstChildElement("communications");
    const auto &windows = root.firstChildElement("windows");
    if (comms.isNull() || windows.isNull()) {
        throw FileLoadException("Invalid file", filename);
    }
    try {
        load_communications_config(comms);
        load_windows(windows);
    } catch (const AppException &e) {
        throw FileLoadException(QString(e), filename);
    }

    const auto &trend = root.firstChildElement("trend");
    if (!trend.isNull()) {
        on_actionTrend_triggered();
        if (!m_trend->load_configuration(trend)) {
            throw FileLoadException("Invalid trend data", filename);
        }
    }
}


void MainWindow::load_communications_config(const QDomElement &node)
{
    const auto common_config = node.firstChildElement("common");
    const auto tcp_config = node.firstChildElement("TCP");
    if (common_config.isNull() || tcp_config.isNull()) {
        throw AppException("Invalid file");
    }

    const auto &method = common_config.attribute("method");
    const auto &timeout = common_config.attribute("timeout");
    if ("TCP" != method) {
        throw AppException("Invalid file");
    }

    if (timeout.toInt() < 1) {
        throw AppException("Invalid file");
    }

    m_ui->timeoutEdit->setText(timeout);

    const auto &port = tcp_config.attribute("port");
    const auto &ip = tcp_config.attribute("ip");
    if (port.toInt() < 1 || port.toInt() > 65535) {
        throw AppException("Invalid file");
    }

    m_ui->portEdit->setText(port);
    m_ui->ipEdit->setText(ip);
    const auto h = node.attribute("h", "-1").toInt();
    const auto w = node.attribute("w", "-1").toInt();
    if (h > 0 && w > 0) {
        resize(w, h);
    }

    const auto x = node.attribute("x", "");
    const auto y = node.attribute("y", "");
    if (x.length() > 0 && y.length() > 0) {
        move(x.toInt(), y.toInt());
    }
}


void MainWindow::load_windows(const QDomNode &node)
{
    const auto &children = node.childNodes();
    for (auto i=0; i<children.count(); ++i) {
        const auto &node = children.at(i);
        if (node.isElement()) {
            const auto &element = node.toElement();
            if (node.nodeName() == "CoilsDisplay") {
                auto config = load_base_data(element);
                auto window = new CoilsDisplay(this,
                                               std::get<1>(config),
                                               std::get<2>(config),
                                               std::get<0>(config));
                if (window->load_configuration_parameters(element)) {
                    add_window(window);
                } else {
                    window->deleteLater();
                }
            } else if (node.nodeName() == "RegisterDisplay") {
                auto config = load_base_data(element);
                auto window = new RegisterDisplay(this,
                                                 std::get<1>(config),
                                                 std::get<2>(config),
                                                 std::get<0>(config));
                if (window->load_configuration_parameters(element)) {
                    add_window(window);
                } else {
                    window->deleteLater();
                }

            } else if (node.nodeName() == "HoldingRegisterDisplay") {
                auto config = load_base_data(element);
                auto window = new HoldingRegisterDisplay(this,
                                                         std::get<1>(config),
                                                         std::get<2>(config),
                                                         std::get<0>(config));
                if (window->load_configuration_parameters(element)) {
                    add_window(window);
                } else {
                    window->deleteLater();
                }

            } else if (node.nodeName() == "InputsDisplay") {
                auto config = load_base_data(element);
                auto window = new InputsDisplay(this,
                                                std::get<1>(config),
                                                std::get<2>(config),
                                                std::get<0>(config));
                if (window->load_configuration_parameters(element)) {
                    add_window(window);
                } else {
                    window->deleteLater();
                }

            } else {
                throw AppException("Invalid file");
            }
        }
    }
}


BaseData MainWindow::load_base_data(const QDomElement &node) const
{
    auto slave_id = node.attribute("node").toInt();
    auto reg = node.attribute("register").toInt();
    auto count = node.attribute("count").toInt();
    auto max = node.attribute("max").toInt();

    if (slave_id < 0 || slave_id > 255 || count < 1 || count > max) {
        throw AppException("Invalid file");
    }

    if ((reg > 0 && reg < 20000) || (reg > 30000 && reg < 50000)) {
        if (((reg + count) / 10000) != (reg / 10000)) {
            throw AppException("Invalid file");
        }
    } else {
        throw AppException("Invalid file");
    }

    return {quint8(slave_id), quint16(reg), quint16(count)};
}


void MainWindow::on_actionRead_Metadata_triggered()
{
    for (auto i: m_register_windows) {
        i->on_refresh_clicked();
    }
}


void MainWindow::on_actionLoad_Register_Data_triggered()
{
    auto file_dialog = QFileDialog(this, tr("Load data..."));
    file_dialog.setNameFilters({tr("Spreadsheet (*.csv)"),
                           tr("All files (*)")});
    file_dialog.setAcceptMode(QFileDialog::AcceptOpen);
    file_dialog.setViewMode(QFileDialog::Detail);
    file_dialog.setFileMode(QFileDialog::ExistingFile);
    if (file_dialog.exec() != 0) {
        auto success = true;
        try {
            const auto filename = file_dialog.selectedFiles().first();
            const auto all_data = QtCSV::Reader::readToList(filename);
            auto csv_dialog = CsvImporter(this, all_data);
            if (csv_dialog.exec() != 0) {
                auto config = csv_dialog.get_config();
                load_csv_data(all_data,
                              std::get<TestFields::REG_VALUE>(config),
                              std::get<TestFields::REG_NUMBER>(config),
                              std::get<TestFields::NODE_ID>(config),
                              std::get<TestFields::HEADER_ROW>(config));
            }
        } catch (const FileLoadException&) {
            success = false;
        } if (success) {
        }
    }
}


void MainWindow::load_csv_data(const QList<QStringList> &all_data,
                               const size_t value_index,
                               const size_t register_index,
                               const ssize_t node_index,
                               const bool skip_first_row)
{
    int fixed_node=0;
    if (node_index < 0) {
        fixed_node = qint16(node_index * -1);
        --fixed_node;
    }

    WriteRequest writes{};

    auto i = all_data.begin();
    if (skip_first_row) {
        ++i;
    }
    for (; all_data.end() != i; ++i) {
        auto regnum = (*i)[int(register_index)].toInt();
        auto value = (*i)[int(value_index)].toInt();
        auto node = (node_index < 0 ? fixed_node : (*i)[node_index].toInt());

        emit m_scheduler->new_register_data(quint16(regnum), quint16(value), quint8(node));
        if ((regnum < 10000 && regnum > 0) || (regnum > 40000 && regnum < 50000)) {
            append_write(writes, quint16(regnum), quint16(value), quint8(node));
        }
    }

    if (writes.values.size() > 0) {
        m_scheduler->modbus_on_write_request(std::move(writes));
    }
}


void MainWindow::append_write(WriteRequest &wr,
                              const quint16 reg,
                              const quint16 value,
                              const quint8 node)
{
    if (wr.values.size() > 0) {
        const auto next_reg = quint16(wr.first_register + wr.values.size());
        const auto max = (reg < 10000 ? 0x7D0U : 127U);
        if (reg != next_reg || node != wr.node || wr.values.size() >= max) {
            m_scheduler->modbus_on_write_request(std::move(wr));
            wr = WriteRequest{};
            wr.first_register = reg;
        }
    } else {
        wr.first_register = reg;
    }

    wr.values.push_back(value);
    wr.node = node;
    wr.requester = nullptr;
}


void MainWindow::on_actionTrend_triggered()
{
    if (nullptr == m_trend) {
        m_ui->actionTrend->setEnabled(false);
        m_trend = new TrendWindow(this);
        connect(m_scheduler, &Scheduler::new_register_data,
                m_trend, &TrendWindow::on_new_value);
        connect(m_trend, &TrendWindow::window_closed,
                this, &MainWindow::trend_on_closed);
        m_trend->show();
    }
}


void MainWindow::trend_on_closed(BaseDialog *w)
{
    if (w == m_trend) {
        disconnect(m_scheduler, &Scheduler::new_register_data,
                   m_trend, &TrendWindow::on_new_value);
        m_trend = nullptr;
        m_ui->actionTrend->setEnabled(true);
    }
}


void MainWindow::modbus_on_error_protocol(const int error_code)
{
    if (m_connecting) {
        post_disconnected();
        m_ui->statusbar->showMessage(tr("Connection failed: %1")
                                     .arg(tr(modbus_strerror(error_code))));
    } else if (m_connecting2) {
        //  Device ID probably unsupported
        m_connecting2=false;
    } else {
    }
}
