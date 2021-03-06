/*
OpenParkingManager - An open source parking manager and parking finder.
    Copyright (C) 2019 Louis van der Walt

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <fstream>
#include <iostream>
#include <string>

#include <base64.hpp>
#include <json.hpp>

#include <LouisNet.hpp>
#include <opencv2/opencv.hpp>

#define PORT 6969

#define MAXDATASIZE 100

using json = nlohmann::json;

std::string disk = "/run/media/louis/178B-7CAB";
std::string ip = "localhost";

int main()
{
    json config;

    std::ifstream config_file("entry_config.json");
    config_file >> config;
    config_file.close();

    ip = config["config"]["ip"];
    disk = config["config"]["disk"];

    while (true) {
        std::cin.get();
        cv::VideoCapture cam(0);
        cv::Mat frame;
        cam >> frame;

        std::vector<uchar> buffer;
        buffer.resize(1024 * 1024 * 10);
        cv::imencode(".jpg", frame, buffer);
        buffer.shrink_to_fit();
        std::string str(buffer.begin(), buffer.end());
        buffer.clear();

        std::string b64str = base64_encode((const unsigned char*)str.c_str(), str.length());

        LouisNet::Packet pack(b64str, 128);
        LouisNet::Socket sock(ip, "6969");
        sock.m_maxsize = 128;
        sock.connect_socket();
        sock.send_data(pack);

        std::cout << sock.receive_data() << std::endl;
        sock.send_data("ENTRY NULL");
        std::ofstream ticket(disk + "/ticket.txt");
        ticket << sock.receive_data();
        ticket.close();
    }

    return 0;
}