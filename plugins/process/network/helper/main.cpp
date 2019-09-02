/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

#include <getopt.h>

#include "Accumulator.h"
#include "Capture.h"
#include "ConnectionMapping.h"
#include "Packet.h"
#include "TimeStamps.h"

static std::atomic_bool g_running{false};

int main(int argc, char **argv)
{
    static struct option long_options[] = {
        {"help",    0, nullptr, 'h'},
        {"stats",   0, nullptr, 's'},
        {nullptr,   0, nullptr, 0}
    };

    auto statsRequested = false;
    auto optionIndex = 0;
    auto option = -1;
    while ((option = getopt_long(argc, argv, "", long_options, &optionIndex)) != -1) {
        switch (option) {
        case 's':
            statsRequested = true;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " [options]\n";
            std::cerr << "This is a helper application for tracking per-process network usage.\n";
            std::cerr << "\n";
            std::cerr << "Options:\n";
            std::cerr << "  --stats     Print packet capture statistics.\n";
            std::cerr << "  --help      Display this help.\n";
            return 0;
        }
    }

    auto mapping = std::make_shared<ConnectionMapping>();

    auto capture = std::make_shared<Capture>();
    if (!capture->start()) {
        std::cerr << capture->lastError() << std::endl;
        return 1;
    }

    auto accumulator = std::make_shared<Accumulator>(capture, mapping);

    signal(SIGINT, [](int) { g_running = false; });
    signal(SIGTERM, [](int) { g_running = false; });

    g_running = true;
    while(g_running) {
        auto data = accumulator->data();
        auto timeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        if (statsRequested != 0) {
            capture->reportStatistics();
        }

        if (data.empty()) {
            std::cout << std::put_time(std::localtime(&timeStamp), "%T") << std::endl;
        } else {
            for (auto itr = data.begin(); itr != data.end(); ++itr) {
                std::cout << std::put_time(std::localtime(&timeStamp), "%T");
                std::cout << "|PID|" << (*itr).first << "|IN|" << (*itr).second.first << "|OUT|" << (*itr).second.second;
                std::cout << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    accumulator->stop();
    capture->stop();

    return 0;
}
