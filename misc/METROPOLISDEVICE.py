import sqlite3
from datetime import datetime, timezone
DB = r'C:/Users*******/Confidential/PD/*******.sqlite'
db = sqlite3.connect(DB); cur = db.cursor()

# Basic stats
print("=" * 70)
print(" SYDNEY-STATION DATASET — INITIAL PROFILE")
print("=" * 70)
print()

cur.execute("SELECT COUNT(*) FROM devices")
n_devices = cur.fetchone()[0]
cur.execute("SELECT COUNT(*) FROM locations")
n_locations = cur.fetchone()[0]
cur.execute("SELECT COUNT(*) FROM user_device_data")
n_user = cur.fetchone()[0]
print(f"Total unique devices:        {n_devices:,}")
print(f"Total detection events:      {n_locations:,}")
print(f"User-tagged devices:         {n_user}")
print()

# Time range — using locations table where timestamps are valid epoch
cur.execute("SELECT MIN(timestamp), MAX(timestamp) FROM locations WHERE timestamp > 1000000000")
mn, mx = cur.fetchone()
if mn and mx:
    dt_mn = datetime.fromtimestamp(mn, tz=timezone.utc)
    dt_mx = datetime.fromtimestamp(mx, tz=timezone.utc)
    span_seconds = mx - mn
    print(f"First detection:             {dt_mn.isoformat()}")
    print(f"Last detection:              {dt_mx.isoformat()}")
    print(f"Span:                        {span_seconds:.0f} seconds = {span_seconds/60:.1f} minutes = {span_seconds/3600:.1f} hours")
    print(f"Average detections/second:   {n_locations/span_seconds:.1f}" if span_seconds else "")
print()

# Geographic spread
cur.execute("""
    SELECT MIN(latitude), MAX(latitude), MIN(longitude), MAX(longitude)
    FROM locations WHERE latitude != 0 AND longitude != 0
""")
lat_mn, lat_mx, lng_mn, lng_mx = cur.fetchone()
if lat_mn and lat_mx:
    import math
    bbox_h_km = (lat_mx - lat_mn) * 111
    bbox_w_km = (lng_mx - lng_mn) * 111 * math.cos(math.radians((lat_mn+lat_mx)/2))
    print(f"GPS bounding box:")
    print(f"  Lat: {lat_mn:.5f} to {lat_mx:.5f}  (~{bbox_h_km:.2f} km tall)")
    print(f"  Lng: {lng_mn:.5f} to {lng_mx:.5f}  (~{bbox_w_km:.2f} km wide)")
    print(f"  Centroid: ({(lat_mn+lat_mx)/2:.4f}, {(lng_mn+lng_mx)/2:.4f})")
print()

# Axon device count
cur.execute("""
    SELECT COUNT(*) FROM devices WHERE upper(address) LIKE '00:25:DF%'
""")
axon_oui = cur.fetchone()[0]
cur.execute("""
    SELECT COUNT(*) FROM devices WHERE address = '00:00:00:00:00:00'
""")
axon_zero = cur.fetchone()[0]

# Devices using Axon FE6B service data
cur.execute("""
    SELECT COUNT(*) FROM devices WHERE service_data LIKE '%fe6b:%'
""")
axon_fe6b = cur.fetchone()[0]

# Devices with the METROPOLISDEVICE 128-bit UUID
cur.execute("""
    SELECT COUNT(*) FROM devices
    WHERE service_data LIKE '%4d455452-4f50-4f4c-4953-444556494345%'
       OR service_uuids LIKE '%4d455452-4f50-4f4c-4953-444556494345%'
""")
axon_metro = cur.fetchone()[0]

# Devices with the 9ec5d2b8 128-bit UUID (docked-mode UUID)
cur.execute("""
    SELECT COUNT(*) FROM devices
    WHERE service_data LIKE '%9ec5d2b8-8f51-4dea-9cd3-f3dea220b5e0%'
       OR service_uuids LIKE '%9ec5d2b8-8f51-4dea-9cd3-f3dea220b5e0%'
""")
axon_9ec5 = cur.fetchone()[0]

print("=== Axon device identification ===")
print(f"  By OUI (00:25:DF):                          {axon_oui}")
print(f"  By zero-MAC (00:00:00:00:00:00):            {axon_zero}")
print(f"  By FE6B service-data UUID:                  {axon_fe6b}")
print(f"  By METROPOLISDEVICE 128-bit UUID:           {axon_metro}")
print(f"  By docked-mode 9ec5d2b8 128-bit UUID:       {axon_9ec5}")
print()

# Detections involving Axon devices
cur.execute("""
    SELECT COUNT(*) FROM locations l
    JOIN devices d ON l.device_id = d.id
    WHERE upper(d.address) LIKE '00:25:DF%' OR d.address = '00:00:00:00:00:00'
       OR d.service_data LIKE '%fe6b:%'
""")
axon_detections = cur.fetchone()[0]
print(f"Total Axon-related detection events:          {axon_detections:,}")
print()

# RaMBLE's device_type identification
cur.execute("""
    SELECT device_type, COUNT(*) FROM devices
    WHERE device_type IS NOT NULL AND device_type != ''
    GROUP BY device_type
    ORDER BY 2 DESC
    LIMIT 15
""")
print("=== Top device-type tags (RaMBLE auto-identified) ===")
for dtype, n in cur.fetchall():
    print(f"  {n:>5}  {dtype}")
