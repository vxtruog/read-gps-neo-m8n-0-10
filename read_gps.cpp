#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

// Hàm chuyển NMEA -> decimal
double nmeaToDecimal(const std::string& nmea, char direction) {
    if (nmea.empty()) return 0.0;
    double raw = 0.0;
    try {
        raw = std::stod(nmea);
    } catch (...) {
        return 0.0; // nếu không phải số, trả 0
    }
    int degrees = int(raw / 100);
    double minutes = raw - degrees * 100;
    double dec = degrees + minutes / 60.0;
    if (direction == 'S' || direction == 'W') dec *= -1;
    return dec;
}

int main() {
    int serial_port = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (serial_port < 0) {
        perror("open");
        return 1;
    }

    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        perror("tcgetattr");
        return 1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    tcflush(serial_port, TCIFLUSH);
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return 1;
    }

    std::string leftover; // buffer nối dòng chưa đủ

    char buf[256];
    while (true) {
        int n = read(serial_port, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            leftover += buf;

            size_t pos;
            while ((pos = leftover.find('\n')) != std::string::npos) {
                std::string line = leftover.substr(0, pos);
                leftover.erase(0, pos + 1);

                // loại bỏ \r cuối dòng
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                if (line.find("$GNRMC") != std::string::npos || line.find("$GNGGA") != std::string::npos) {
                    std::vector<std::string> fields;
                    std::stringstream ss(line);
                    std::string field;
                    while (std::getline(ss, field, ',')) {
                        fields.push_back(field);
                    }

                    // kiểm tra đủ trường lat/lon
                    if (fields.size() >= 6 && !fields[2].empty() && !fields[4].empty()) {
                        char lat_dir = fields[3].empty() ? 'N' : fields[3][0];
                        char lon_dir = fields[5].empty() ? 'E' : fields[5][0];
                        double lat = nmeaToDecimal(fields[2], lat_dir);
                        double lon = nmeaToDecimal(fields[4], lon_dir);

                        std::ofstream gps_out("/home/pi/gps.txt");
                        gps_out << std::fixed << std::setprecision(8) << lat << "," << lon << std::endl;
                        gps_out.flush();
                    }
                }
            }
        }
    }

    close(serial_port);
    return 0;
}
