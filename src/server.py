"""
ESP32 + LoRa Weather Station - Backend Server
Tuong thich voi Gateway dung HTTP GET:
  ?temp=36.0&pressure=101325.0&hum=72.5&lux=450&rain=85&status=mua_nhe

Chay:
    pip install fastapi uvicorn
    python server.py

API Docs: http://localhost:8000/docs
"""

from fastapi import FastAPI, Query, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse, JSONResponse
import sqlite3, time, math, os

app = FastAPI(title="ESP32 Weather API", version="3.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

DB_PATH = "weather.db"

# ─── DATABASE ─────────────────────────────────
def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS readings (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id   TEXT    DEFAULT 'ESP32-NODE-01',
            temp      REAL    NOT NULL,
            pressure  REAL    NOT NULL,
            humidity  REAL    NOT NULL,
            lux       INTEGER DEFAULT 0,
            rain_raw  INTEGER DEFAULT 1023,
            rain_status TEXT  DEFAULT 'khong_mua',
            timestamp INTEGER NOT NULL
        )
    """)
    conn.commit()
    conn.close()

init_db()

def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

# ─── KHÍ TƯỢNG ────────────────────────────────
def calc_heat_index(T, RH):
    F = T * 9/5 + 32
    HI = (-42.379 + 2.04901523*F + 10.14333127*RH
          - 0.22475541*F*RH - 0.00683783*F**2
          - 0.05481717*RH**2 + 0.00122874*F**2*RH
          + 0.00085282*F*RH**2 - 0.00000199*F**2*RH**2)
    return round((HI - 32) * 5/9, 1)

def calc_dew_point(T, RH):
    a, b = 17.27, 237.7
    alpha = (a * T) / (b + T) + math.log(max(RH, 0.1) / 100)
    return round(b * alpha / (a - alpha), 1)

def get_pressure_trend(conn):
    """hPa/gio trong 3 gio gan nhat"""
    cutoff = int(time.time()) - 3 * 3600
    rows = conn.execute(
        "SELECT pressure, timestamp FROM readings WHERE timestamp > ? ORDER BY timestamp",
        (cutoff,)
    ).fetchall()
    if len(rows) < 2:
        return 0.0
    dp = rows[-1]["pressure"] - rows[0]["pressure"]
    dt = (rows[-1]["timestamp"] - rows[0]["timestamp"]) / 3600
    return round(dp / dt, 2) if dt > 0.01 else 0.0

def forecast_weather(temp, humidity, pressure, trend, rain_status, lux):
    is_rain = rain_status in ("mua_to", "mua_nhe")

    if rain_status == "mua_to":
        return {"condition": "Mưa to", "icon": "⛈",
                "confidence": 95,
                "description": "Cảm biến phát hiện mưa to (rain < 400)."}
    if rain_status == "mua_nhe":
        if trend < -1:
            return {"condition": "Mưa nhẹ, có thể tăng", "icon": "🌧",
                    "confidence": 85,
                    "description": "Đang mưa nhẹ, áp suất đang giảm."}
        return {"condition": "Mưa nhẹ", "icon": "🌦",
                "confidence": 82,
                "description": "Cảm biến phát hiện mưa nhẹ (400 ≤ rain < 800)."}
    # khong_mua
    if trend < -3 and humidity > 75:
        return {"condition": "Sắp có giông", "icon": "🌩",
                "confidence": 80,
                "description": "Áp suất giảm rất mạnh, độ ẩm cao."}
    if trend < -1.5 and humidity > 65:
        return {"condition": "Khả năng mưa", "icon": "🌥",
                "confidence": 70,
                "description": "Áp suất đang giảm dần."}
    if humidity > 80 and lux < 200:
        return {"condition": "Trời âm u", "icon": "☁️",
                "confidence": 65,
                "description": "Độ ẩm rất cao, ít ánh sáng."}
    if pressure > 1015 and humidity < 55 and lux > 1000:
        return {"condition": "Nắng đẹp", "icon": "☀️",
                "confidence": 91,
                "description": "Áp suất cao, trời nắng sáng."}
    if lux < 500 and humidity > 60:
        return {"condition": "Có mây", "icon": "⛅",
                "confidence": 62,
                "description": "Ánh sáng yếu, độ ẩm trung bình."}
    return {"condition": "Quang mây", "icon": "🌤",
            "confidence": 68,
            "description": "Thời tiết ổn định."}

# ─── ENDPOINT GATEWAY GỌI (HTTP GET) ─────────

@app.get("/update.php", summary="Gateway goi bang HTTP GET")
def update(
    temp:     float = Query(..., description="Nhiet do C"),
    pressure: float = Query(..., description="Ap suat (Pa hoac hPa)"),
    hum:      float = Query(..., description="Do am %"),
    lux:      int   = Query(..., description="Cuong do anh sang lux"),
    rain:     int   = Query(..., description="Gia tri analog 0-1023"),
    status:   str   = Query(..., description="khong_mua / mua_nhe / mua_to"),
    node_id:  str   = Query("ESP32-NODE-01", description="ID cua node"),
):
    """
    ESP32 Gateway goi endpoint nay bang HTTP GET:
    GET /update.php?temp=36&pressure=101325&hum=72&lux=450&rain=350&status=mua_nhe

    Luu y: pressure tu Node la Pa (vi du 101325),
    server tu dong chia 100 neu > 2000 de chuyen sang hPa.
    """
    # Tự động chuẩn hóa pressure: nếu > 2000 thì đang là Pa → chia 100
    pressure_hpa = pressure / 100.0 if pressure > 2000 else pressure

    conn = get_db()
    conn.execute(
        """INSERT INTO readings
           (node_id, temp, pressure, humidity, lux, rain_raw, rain_status, timestamp)
           VALUES (?,?,?,?,?,?,?,?)""",
        (node_id, temp, pressure_hpa, hum, lux, rain, status, int(time.time()))
    )
    conn.commit()
    conn.close()

    ts = time.strftime('%H:%M:%S')
    print(f"[{ts}] {node_id} | T={temp}C P={pressure_hpa:.1f}hPa "
          f"H={hum}% Lux={lux} Rain={rain}({status})")

    return {"status": "ok", "pressure_hpa": round(pressure_hpa, 2)}


# ─── ENDPOINTS CHO DASHBOARD ──────────────────

@app.get("/api/latest")
def get_latest():
    conn = get_db()
    row = conn.execute(
        "SELECT * FROM readings ORDER BY timestamp DESC LIMIT 1"
    ).fetchone()
    if not row:
        raise HTTPException(status_code=404, detail="Chua co du lieu")
    trend = get_pressure_trend(conn)
    conn.close()

    T, RH, P = row["temp"], row["humidity"], row["pressure"]
    rain_status = row["rain_status"]
    lux = row["lux"]

    fc = forecast_weather(T, RH, P, trend, rain_status, lux)
    return {
        "node_id":      row["node_id"],
        "temp":         T,
        "humidity":     RH,
        "pressure":     P,
        "lux":          lux,
        "rain_raw":     row["rain_raw"],
        "rain_status":  rain_status,
        "timestamp":    row["timestamp"],
        "heat_index":   calc_heat_index(T, RH),
        "dew_point":    calc_dew_point(T, RH),
        "press_trend":  trend,
        "forecast":     fc,
    }


@app.get("/api/history")
def get_history(hours: int = 6, node_id: str = "ESP32-NODE-01"):
    cutoff = int(time.time()) - hours * 3600
    conn = get_db()
    rows = conn.execute(
        "SELECT * FROM readings WHERE timestamp > ? AND node_id = ? ORDER BY timestamp",
        (cutoff, node_id)
    ).fetchall()
    conn.close()
    return [dict(r) for r in rows]


@app.get("/api/stats")
def get_stats(hours: int = 24):
    cutoff = int(time.time()) - hours * 3600
    conn = get_db()
    row = conn.execute("""
        SELECT
          ROUND(MIN(temp),1) temp_min, ROUND(MAX(temp),1) temp_max, ROUND(AVG(temp),1) temp_avg,
          ROUND(MIN(humidity),1) humi_min, ROUND(MAX(humidity),1) humi_max, ROUND(AVG(humidity),1) humi_avg,
          ROUND(MIN(pressure),1) pres_min, ROUND(MAX(pressure),1) pres_max, ROUND(AVG(pressure),1) pres_avg,
          MAX(lux) lux_max, ROUND(AVG(lux),0) lux_avg,
          SUM(CASE WHEN rain_status != 'khong_mua' THEN 1 ELSE 0 END) rain_count,
          COUNT(*) total
        FROM readings WHERE timestamp > ?
    """, (cutoff,)).fetchone()
    conn.close()
    return dict(row)


@app.get("/api/nodes")
def get_nodes():
    conn = get_db()
    rows = conn.execute("""
        SELECT node_id, MAX(timestamp) last_seen,
               COUNT(*) packet_count
        FROM readings GROUP BY node_id
    """).fetchall()
    conn.close()
    now = int(time.time())
    return [{**dict(r),
             "status": "online" if (now - r["last_seen"]) < 120 else "offline"}
            for r in rows]


# Serve dashboard
if os.path.exists("index.html"):
    @app.get("/", include_in_schema=False)
    def serve_dashboard():
        return FileResponse("index.html")


if __name__ == "__main__":
    import uvicorn
    ip = "localhost"
    print("\n" + "="*60)
    print("  ESP32 LoRa Weather Server - v3.0")
    print(f"  Dashboard  ->  http://{ip}:8000")
    print(f"  API Docs   ->  http://{ip}:8000/docs")
    print()
    print("  Gateway ESP32 chi can sua 1 dong:")
    print(f'  String server = "http://IP_MAY_TINH:8000/update.php";')
    print("="*60 + "\n")
    uvicorn.run("server:app", host="0.0.0.0", port=8000, reload=True)
