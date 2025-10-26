# Cấu hình sử dụng module GPS NEO-M8N-0-10 trên Raspberry Pi 4
- Vào phần mềm u-center để cấu hình baudrate và các dữ liệu cần đọc cho module gps m8n.

- `sudo nano /boot/firmware/config.txt` và thêm
```
dtoverlay=disable-bt
core_freq=250
enable_uart=1
force_turbo=1
```

- `sudo nano /boot/cmdline.txt` và thêm
```
dwc_otg.lpm_enable=0 console=tty1 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait quiet splash plymouth.ignore-serial-consoles
```

- Chạy các lệnh sau và chạy sau mỗi lần khởi động hoặc bật nguồn Raspberry Pi 4
```
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service

sudo systemctl stop serial-getty@ttys0.service
sudo systemctl disable serial-getty@ttys0.service

sudo systemctl enable serial-getty@ttys0.service
```

- Cấu hình baudrate phù hợp với cấu hình module GPS NEO-M8N-0-10 đã cài đặt trong u-center và đọc dữ liệu
```
sudo stty -F /dev/serial0 115200
sudo cat /dev/serial0
```
