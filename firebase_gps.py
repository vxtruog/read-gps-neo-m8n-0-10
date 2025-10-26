import firebase_admin
from firebase_admin import credentials, db
import time
import os

# ==== 1. Kết nối Firebase ====
cred = credentials.Certificate("/home/pi/firebase_key.json")  # đường dẫn key JSON
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://your-project-id-default-rtdb.firebaseio.com/'
})

# ==== 2. Đường dẫn tới file GPS do C++ ghi ====
gps_file = "/home/pi/gps.txt"

# ==== 3. Vòng lặp đọc file và đẩy Firebase ====
ref = db.reference("gps_data")  # nút gốc trong database

print("Đang theo dõi file GPS và đẩy lên Firebase...")

last_data = ""
while True:
    if os.path.exists(gps_file):
        with open(gps_file, "r") as f:
            data = f.readline().strip()

        # Nếu dữ liệu mới khác dữ liệu cũ → push Firebase
        if data and data != last_data:
            try:
                lat, lon = map(float, data.split(","))
                ref.set({
                    "latitude": lat,
                    "longitude": lon,
                    "timestamp": time.strftime("%Y-%m-%d %H:%M:%S")
                })
                print(f"Gửi lên Firebase: {lat}, {lon}")
                last_data = data
            except ValueError:
                pass

    time.sleep(1)  # đọc lại mỗi giây
