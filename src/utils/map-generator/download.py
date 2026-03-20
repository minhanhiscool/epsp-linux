import math
import gzip
import requests
from PIL import Image, ImageDraw
import mapbox_vector_tile  # pip install mapbox-vector-tile Pillow requests

# ── Config ────────────────────────────────────────────────────────────────────
KEY = "LMkbV8iByBD40zIsjeNc"  # ← your MapTiler API key
ZOOM = 6
TILE_SIZE = 1024  # pixels per tile

# Bounding box
LAT_MIN, LAT_MAX = 23.0, 47.0
LON_MIN, LON_MAX = 121.0, 150.0

# ── Style colours (from your style.json) ──────────────────────────────────────
BG_COLOR = (15, 20, 35)  # background layer (dark navy)
WATER_COLOR = (42, 54, 76)  # water fill
BORDER_COLOR = (255, 255, 255)  # country border (admin_level=2)
BORDER_WIDTH = 2
PREF_COLOR = (153, 153, 153)  # prefecture border (admin_level=4)
PREF_WIDTH = 1


# ── Helpers ───────────────────────────────────────────────────────────────────
def latlon_to_tile(lat, lon, z):
    n = 2**z
    x = int((lon + 180.0) / 360.0 * n)
    lat_rad = math.radians(lat)
    y = int(
        (1.0 - math.log(math.tan(lat_rad) + 1 / math.cos(lat_rad)) / math.pi) / 2.0 * n
    )
    return x, y


def mvt_coord_to_px(coord, extent=4096):
    """Convert MVT tile coordinate (0–extent) to pixel coordinate (0–TILE_SIZE)."""
    px = coord[0] / extent * TILE_SIZE
    py = TILE_SIZE - (coord[1] / extent * TILE_SIZE)  # flip y
    return (px, py)


def render_tile(pbf_bytes):
    """Parse a PBF vector tile and render it to a PIL Image."""
    # Decompress if gzip-encoded
    try:
        pbf_bytes = gzip.decompress(pbf_bytes)
    except (OSError, gzip.BadGzipFile):
        pass

    tile_data = mapbox_vector_tile.decode(pbf_bytes)

    img = Image.new("RGB", (TILE_SIZE, TILE_SIZE), BG_COLOR)
    draw = ImageDraw.Draw(img)

    # ── 1. Water fill ─────────────────────────────────────────────────────────
    # Pillow does not support polygon holes natively.
    # Fix: fill the outer ring with WATER_COLOR, then paint each inner ring
    # (hole = land cutout) back with BG_COLOR.
    if "water" in tile_data:
        for feat in tile_data["water"]["features"]:
            geom = feat["geometry"]
            gtype = geom["type"]

            if gtype == "Polygon":
                polygons = [geom["coordinates"]]
            elif gtype == "MultiPolygon":
                polygons = geom["coordinates"]
            else:
                continue

            for poly in polygons:
                if not poly:
                    continue
                # Outer ring → water
                outer_pts = [mvt_coord_to_px(c) for c in poly[0]]
                if len(outer_pts) >= 3:
                    draw.polygon(outer_pts, fill=WATER_COLOR)
                # Inner rings (holes) → land (background colour)
                for hole in poly[1:]:
                    hole_pts = [mvt_coord_to_px(c) for c in hole]
                    if len(hole_pts) >= 3:
                        draw.polygon(hole_pts, fill=BG_COLOR)

    # ── 2. Boundaries ─────────────────────────────────────────────────────────
    if "boundary" in tile_data:
        for feat in tile_data["boundary"]["features"]:
            props = feat.get("properties", {})
            admin = props.get("admin_level", 0)
            maritime = props.get("maritime", 0)

            if maritime == 1:
                continue  # skip maritime boundaries (matches your style filter)

            if admin == 2:
                colour, width = BORDER_COLOR, BORDER_WIDTH
            elif admin == 4:
                colour, width = PREF_COLOR, PREF_WIDTH
            else:
                continue

            geom = feat["geometry"]
            gtype = geom["type"]

            if gtype == "LineString":
                lines = [geom["coordinates"]]
            elif gtype == "MultiLineString":
                lines = geom["coordinates"]
            else:
                continue

            for line in lines:
                pts = [mvt_coord_to_px(c) for c in line]
                if len(pts) >= 2:
                    draw.line(pts, fill=colour, width=width)

    return img


# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    # Convert bbox to tile ranges
    # Note: lat_min → y_max  and  lat_max → y_min  (y axis is flipped in tile coords)
    x_min, y_max = latlon_to_tile(LAT_MIN, LON_MIN, ZOOM)
    x_max, y_min = latlon_to_tile(LAT_MAX, LON_MAX, ZOOM)

    cols = x_max - x_min + 1
    rows = y_max - y_min + 1
    print(f"Tile grid: {cols} × {rows}  ({cols * rows} tiles)")

    canvas = Image.new("RGB", (cols * TILE_SIZE, rows * TILE_SIZE), BG_COLOR)

    for x in range(x_min, x_max + 1):
        for y in range(y_min, y_max + 1):
            url = (
                f"https://api.maptiler.com/tiles/v3-openmaptiles"
                f"/{ZOOM}/{x}/{y}.pbf?key={KEY}"
            )
            print(f"  Downloading z{ZOOM}/{x}/{y} ...", end=" ")

            resp = requests.get(url, timeout=15)
            if resp.status_code != 200:
                print(f"SKIP (HTTP {resp.status_code})")
                continue

            tile_img = render_tile(resp.content)

            # Paste into canvas at the correct position
            px = (x - x_min) * TILE_SIZE
            py = (y - y_min) * TILE_SIZE
            canvas.paste(tile_img, (px, py))
            print("OK")

    out_path = f"output_z{ZOOM}.png"
    canvas.save(out_path)
    print(f"\nSaved → {out_path}  ({canvas.width}×{canvas.height} px)")


if __name__ == "__main__":
    main()
