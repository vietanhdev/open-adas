#!/usr/bin/env python3
"""
generate_sim_videos.py
Generates synthetic road-scene videos for open-adas simulation scenarios.
Pure OpenCV/NumPy — no internet access required.

Scenes rendered:
  - Perspective road with lane markings
  - Moving vehicles ahead (cars, trucks)
  - Sky gradient, roadside trees/poles
  - Scenario-specific behaviour (collision, lane departure, speed limit signs)
"""

import cv2
import numpy as np
import math
import os
import random
import sys

W, H = 1280, 720
FPS  = 30

# ── Vanishing point (horizon) ─────────────────────────────────────────────────
VPX, VPY = W // 2, int(H * 0.40)   # vanishing point (x, y)

# ── Road geometry helpers ──────────────────────────────────────────────────────

def lerp(a, b, t):
    return a + (b - a) * t

def road_x(lane_offset_norm, depth):
    """
    Convert (lateral offset in [-1,1] road-width space, depth in [0,1])
    to pixel (x, y).
    depth=0 → bottom of screen, depth=1 → vanishing point
    """
    road_half_bottom = W * 0.40          # half road width at bottom
    road_half_top    = W * 0.04          # half road width at horizon
    y = lerp(H, VPY, depth)
    half = lerp(road_half_bottom, road_half_top, depth)
    x = VPX + lane_offset_norm * half
    return int(x), int(y)

# ── Drawing primitives ─────────────────────────────────────────────────────────

def draw_sky(frame, t):
    """Gradient sky with moving clouds."""
    for row in range(VPY + 30):
        alpha = row / (VPY + 30)
        r = int(lerp(100, 180, alpha))
        g = int(lerp(140, 210, alpha))
        b = int(lerp(200, 240, alpha))
        frame[row, :] = (r, g, b)
    # simple cloud blobs
    rng = random.Random(int(t * 0.2))
    for _ in range(4):
        cx = int(rng.uniform(0, W))
        cy = int(rng.uniform(20, VPY - 30))
        for dx in range(-60, 61, 20):
            cv2.circle(frame, (cx + dx, cy), 22,
                       (220, 225, 240), -1, cv2.LINE_AA)
        cv2.circle(frame, (cx, cy - 12), 28, (230, 235, 245), -1, cv2.LINE_AA)

def draw_road(frame):
    """Fill trapezoid road polygon."""
    bl = road_x(-1, 0)
    br = road_x( 1, 0)
    tl = road_x(-1, 1)
    tr = road_x( 1, 1)
    pts = np.array([bl, br, tr, tl], np.int32)
    cv2.fillPoly(frame, [pts], (60, 60, 65))   # asphalt dark gray

def draw_lane_markings(frame, offset=0.0):
    """Draw dashed center line and solid edge lines, with lateral offset for LDW."""
    # solid left edge
    for d in np.linspace(0.01, 0.99, 80):
        p1 = road_x(-1 + offset, d)
        p2 = road_x(-1 + offset, d + 0.015)
        w  = max(1, int(3 * (1 - d)))
        cv2.line(frame, p1, p2, (220, 220, 220), w, cv2.LINE_AA)
    # solid right edge
    for d in np.linspace(0.01, 0.99, 80):
        p1 = road_x(1 + offset, d)
        p2 = road_x(1 + offset, d + 0.015)
        w  = max(1, int(3 * (1 - d)))
        cv2.line(frame, p1, p2, (220, 220, 220), w, cv2.LINE_AA)
    # dashed center
    dash_period = 0.08
    for start in np.arange(0.01, 0.99, dash_period):
        end = start + dash_period * 0.5
        if end > 0.99:
            break
        p1 = road_x(0 + offset, start)
        p2 = road_x(0 + offset, end)
        w  = max(1, int(2 * (1 - start)))
        cv2.line(frame, p1, p2, (240, 220, 80), w, cv2.LINE_AA)

def draw_roadside(frame, scroll):
    """Trees and poles on the sides."""
    rng = random.Random(42)
    positions = [rng.uniform(0.05, 0.90) for _ in range(12)]
    for pos in positions:
        d = (pos + scroll * 0.03) % 0.92 + 0.04
        # left side tree
        tx, ty = road_x(-1.18, d)
        trunk_h = int(lerp(60, 12, d))
        trunk_w = max(1, int(lerp(8, 2, d)))
        cv2.rectangle(frame,
                      (tx - trunk_w//2, ty - trunk_h),
                      (tx + trunk_w//2, ty),
                      (80, 55, 30), -1)
        crown_r = int(lerp(40, 8, d))
        cv2.circle(frame, (tx, ty - trunk_h), crown_r, (34, 100, 34), -1, cv2.LINE_AA)
        # right side tree
        tx2, ty2 = road_x(1.18, d)
        cv2.rectangle(frame,
                      (tx2 - trunk_w//2, ty2 - trunk_h),
                      (tx2 + trunk_w//2, ty2),
                      (80, 55, 30), -1)
        cv2.circle(frame, (tx2, ty2 - trunk_h), crown_r, (34, 100, 34), -1, cv2.LINE_AA)

def draw_vehicle(frame, depth, lateral=0.0, color=(30, 50, 180), is_truck=False):
    """Draw a car or truck at a given depth [0.05 … 0.95]."""
    cx, cy = road_x(lateral, depth)
    car_w  = int(lerp(260, 10, depth))
    car_h  = int(lerp(120, 6,  depth))
    if is_truck:
        car_w = int(car_w * 1.4)
        car_h = int(car_h * 1.6)
    x1, y1 = cx - car_w//2, cy - car_h
    x2, y2 = cx + car_w//2, cy
    # body
    cv2.rectangle(frame, (x1, y1), (x2, y2), color, -1)
    # roof (lighter)
    roof_x1 = x1 + car_w // 5
    roof_x2 = x2 - car_w // 5
    roof_y1 = y1 - car_h // 3
    if not is_truck:
        cv2.rectangle(frame, (roof_x1, roof_y1), (roof_x2, y1),
                      tuple(min(c + 40, 255) for c in color), -1)
    # windshield
    wsh_margin = car_w // 6
    cv2.rectangle(frame,
                  (x1 + wsh_margin, y1 + car_h//8),
                  (x2 - wsh_margin, y1 + car_h//2),
                  (180, 200, 210), -1)
    # brake lights
    light_h = car_h // 4
    cv2.rectangle(frame, (x1, y2 - light_h), (x1 + car_w//6, y2), (20, 20, 200), -1)
    cv2.rectangle(frame, (x2 - car_w//6, y2 - light_h), (x2, y2), (20, 20, 200), -1)
    # return bounding box for reference
    return (x1, y1, x2, y2)

def draw_speed_sign(frame, speed_kmh, corner=(40, 40)):
    """Draw a circular speed limit sign."""
    cx, cy = corner
    r = 38
    cv2.circle(frame, (cx, cy), r,    (255, 255, 255), -1, cv2.LINE_AA)
    cv2.circle(frame, (cx, cy), r,    (0, 0, 220),     4,  cv2.LINE_AA)
    cv2.circle(frame, (cx, cy), r-4,  (0, 0, 220),     3,  cv2.LINE_AA)
    txt = str(speed_kmh)
    fs  = 0.9 if speed_kmh < 100 else 0.7
    tsz = cv2.getTextSize(txt, cv2.FONT_HERSHEY_DUPLEX, fs, 2)[0]
    tx  = cx - tsz[0]//2
    ty  = cy + tsz[1]//2
    cv2.putText(frame, txt, (tx, ty),
                cv2.FONT_HERSHEY_DUPLEX, fs, (0, 0, 0), 2, cv2.LINE_AA)

def draw_hud(frame, speed_kmh, frame_idx, scenario_name):
    """Minimal HUD overlay."""
    cv2.rectangle(frame, (0, H-36), (280, H), (0, 0, 0), -1)
    cv2.putText(frame, f"Speed: {speed_kmh} km/h  |  Frame {frame_idx}",
                (8, H - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.55,
                (200, 255, 200), 1, cv2.LINE_AA)

# ── Scenario renderers ─────────────────────────────────────────────────────────

def render_collision_scenario(out_path, n_frames, scenario_speed_profile, seed=0):
    """
    A car approaches from ahead; at some point it brakes hard → collision warning.
    """
    rng = random.Random(seed)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    vw = cv2.VideoWriter(out_path, fourcc, FPS, (W, H))

    car_depth  = 0.70    # start far
    car_lat    = rng.uniform(-0.12, 0.12)
    car_color  = [(30,60,200),(200,50,40),(40,160,40),(180,120,30)][seed % 4]
    brake_frame = int(n_frames * 0.45)

    for i in range(n_frames):
        frame = np.zeros((H, W, 3), np.uint8)
        t = i / FPS
        draw_sky(frame, t)
        draw_road(frame)
        draw_roadside(frame, i * 0.015)
        draw_lane_markings(frame)

        # vehicle approaches
        if i < brake_frame:
            car_depth -= 0.00035 * (1 + i / brake_frame)
        else:
            # braking: slow closure
            car_depth -= 0.00008

        car_depth = max(0.08, car_depth)
        draw_vehicle(frame, car_depth, car_lat, car_color)

        speed = scenario_speed_profile[min(i, len(scenario_speed_profile)-1)]
        draw_hud(frame, speed, i, "collision")

        # collision warning red overlay when very close
        if car_depth < 0.25:
            overlay = frame.copy()
            cv2.rectangle(overlay, (0,0), (W, H), (0, 0, 180), -1)
            cv2.addWeighted(frame, 0.85, overlay, 0.15, 0, frame)
            cv2.putText(frame, "!! COLLISION WARNING !!",
                        (W//2 - 220, 80),
                        cv2.FONT_HERSHEY_DUPLEX, 1.2, (0, 0, 255), 3, cv2.LINE_AA)

        vw.write(frame)

    vw.release()
    print(f"  [+] {out_path} ({n_frames} frames)")

def render_lane_departure(out_path, n_frames, seed=1):
    """
    Car drifts laterally → lane departure warning.
    """
    rng = random.Random(seed)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    vw = cv2.VideoWriter(out_path, fourcc, FPS, (W, H))

    drift_start = int(n_frames * 0.30)
    drift_end   = int(n_frames * 0.65)

    for i in range(n_frames):
        frame = np.zeros((H, W, 3), np.uint8)
        t = i / FPS

        # lateral camera shift (ego car drifting)
        if drift_start <= i <= drift_end:
            progress = (i - drift_start) / (drift_end - drift_start)
            lane_offset = math.sin(progress * math.pi) * 0.40
        else:
            lane_offset = 0.0

        draw_sky(frame, t)
        draw_road(frame)
        draw_roadside(frame, i * 0.015)
        draw_lane_markings(frame, offset=-lane_offset)

        # a slow car ahead stays put
        draw_vehicle(frame, 0.55, 0.05, (120, 60, 200))

        draw_hud(frame, 70, i, "LDW")

        if drift_start <= i <= drift_end and lane_offset > 0.15:
            cv2.putText(frame, "!! LANE DEPARTURE WARNING !!",
                        (W//2 - 260, 80),
                        cv2.FONT_HERSHEY_DUPLEX, 1.1, (0, 200, 255), 3, cv2.LINE_AA)

        vw.write(frame)

    vw.release()
    print(f"  [+] {out_path} ({n_frames} frames)")

def render_speed_limit(out_path, n_frames, limit_kmh=60, seed=2):
    """
    Normal driving scene, speed sign visible, speed changes over time.
    """
    rng = random.Random(seed)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    vw = cv2.VideoWriter(out_path, fourcc, FPS, (W, H))

    # two cars at different depths
    c1_color = [(30,60,200),(180,60,30)][seed%2]
    c2_color = (60, 160, 60)

    for i in range(n_frames):
        frame = np.zeros((H, W, 3), np.uint8)
        t = i / FPS

        # oscillating speeds
        phase      = (i / n_frames) * 2 * math.pi
        cur_speed  = int(limit_kmh * 0.8 + 30 * math.sin(phase * 1.3))
        cur_speed  = max(0, cur_speed)

        draw_sky(frame, t)
        draw_road(frame)
        draw_roadside(frame, i * 0.015)
        draw_lane_markings(frame)

        # slow car far ahead
        depth1 = 0.60 + 0.06 * math.sin(phase * 0.5)
        draw_vehicle(frame, depth1, 0.0, c1_color)

        # oncoming in opposite lane (just visible right side)
        depth2 = (i * 0.003) % 0.85 + 0.10
        draw_vehicle(frame, 1.0 - depth2, 0.85, c2_color)

        draw_speed_sign(frame, limit_kmh, (W - 80, 80))
        draw_hud(frame, cur_speed, i, f"speed_limit_{limit_kmh}")

        if cur_speed > limit_kmh + 5:
            cv2.putText(frame, f"!! OVERSPEED: {cur_speed} km/h !!",
                        (W//2 - 200, 80),
                        cv2.FONT_HERSHEY_DUPLEX, 1.0, (0, 80, 255), 3, cv2.LINE_AA)

        vw.write(frame)

    vw.release()
    print(f"  [+] {out_path} ({n_frames} frames)")

def render_calib(out_path, n_frames):
    """
    Slow drive with a flat homogeneous road for camera calibration.
    Includes a calibration carpet pattern on the road.
    """
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    vw = cv2.VideoWriter(out_path, fourcc, FPS, (W, H))

    for i in range(n_frames):
        frame = np.zeros((H, W, 3), np.uint8)
        t = i / FPS
        draw_sky(frame, t)
        draw_road(frame)

        # checkerboard on road surface for calibration
        scroll = i * 0.008
        for row in np.linspace(0.05, 0.80, 14):
            for col in np.linspace(-0.80, 0.80, 10):
                rs = (row + scroll) % 0.88
                parity = (int((row + scroll) * 7) + int((col + 0.90) * 5)) % 2
                color = (200, 200, 200) if parity == 0 else (50, 50, 50)
                cx, cy = road_x(col, rs)
                size = max(2, int(lerp(18, 2, rs)))
                cv2.rectangle(frame,
                              (cx - size, cy - size),
                              (cx + size, cy + size),
                              color, -1)

        draw_lane_markings(frame)
        draw_roadside(frame, i * 0.008)
        draw_hud(frame, 10, i, "calib")
        vw.write(frame)

    vw.release()
    print(f"  [+] {out_path} ({n_frames} frames)")

# ── Speed profiles (match sim_data CarSpeed entries roughly) ──────────────────

def flat_speed(n, kmh):
    return [kmh] * n

def ramp_speed(n, start_kmh, end_kmh):
    return [int(lerp(start_kmh, end_kmh, i/n)) for i in range(n)]

# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    os.makedirs("data/videos", exist_ok=True)

    print("=" * 60)
    print("  OpenADAS Synthetic Video Generator")
    print("=" * 60)

    tasks = [
        # (output_path,                                  func, args)
        ("data/videos/collision_1.mp4",
         render_collision_scenario,
         dict(n_frames=1164, scenario_speed_profile=ramp_speed(1164, 0, 60), seed=0)),

        ("data/videos/collision_2.mp4",
         render_collision_scenario,
         dict(n_frames=1175, scenario_speed_profile=ramp_speed(1175, 20, 80), seed=1)),

        ("data/videos/max_speed_60.mp4",
         render_speed_limit,
         dict(n_frames=589, limit_kmh=60, seed=2)),

        ("data/videos/lane_departure_warning_1.mp4",
         render_lane_departure,
         dict(n_frames=948, seed=3)),

        ("data/videos/sl_60_collision.mp4",
         render_collision_scenario,
         dict(n_frames=589, scenario_speed_profile=flat_speed(589, 70), seed=4)),

        ("data/videos/camera_calib_video.mp4",
         render_calib,
         dict(n_frames=3001)),

        ("data/videos/over_speed.mp4",
         render_speed_limit,
         dict(n_frames=868, limit_kmh=60, seed=5)),
    ]

    for out_path, func, kwargs in tasks:
        print(f"\n[Generating] {out_path}")
        try:
            func(out_path, **kwargs)
        except Exception as e:
            print(f"  [!] ERROR: {e}")
            import traceback; traceback.print_exc()

    # Update sim_list.txt
    sim_list = """7
data/videos/collision_1.mp4
data/sim_data/collision_1.txt
data/videos/collision_2.mp4
data/sim_data/collision_2.txt
data/videos/max_speed_60.mp4
data/sim_data/max_speed_60.txt
data/videos/lane_departure_warning_1.mp4
data/sim_data/LDW_1.txt
data/videos/sl_60_collision.mp4
data/sim_data/sl_60_collision.txt
data/videos/camera_calib_video.mp4
data/sim_data/camera_calib_video.txt
data/videos/over_speed.mp4
data/sim_data/over_speed.txt
"""
    with open("data/sim_list.txt", "w") as f:
        f.write(sim_list)
    print("\n[+] Updated data/sim_list.txt")

    print("\n" + "=" * 60)
    print("  All videos generated in data/videos/")
    print("  Run: ./build/bin/OpenADAS --input_source simulation")
    print("=" * 60)

if __name__ == "__main__":
    main()
