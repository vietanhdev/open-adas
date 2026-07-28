#!/usr/bin/env python3
"""
prepare_videos.py
Downloads a free royalty-free driving video from Wikimedia Commons and
generates simulation video files for all open-adas scenarios.

Source: https://commons.wikimedia.org/wiki/File:Veok_p6leng_pikk.webm
License: Creative Commons Attribution-ShareAlike 4.0
Description: Estonian truck dashcam footage (48 sec, 1920×1080, 25fps)
"""

import subprocess
import os
import sys

# ── Config ────────────────────────────────────────────────────────────────────

# Public-domain / CC driving videos from Wikimedia Commons (direct download URLs)
SOURCE_URLS = [
    # Truck dashcam Estonia ~48s 1080p (CC BY-SA 4.0)
    "https://upload.wikimedia.org/wikipedia/commons/transcoded/4/49/Veok_p6leng_pikk.webm/Veok_p6leng_pikk.webm.1080p.vp9.webm",
    # Fallback 480p
    "https://upload.wikimedia.org/wikipedia/commons/transcoded/4/49/Veok_p6leng_pikk.webm/Veok_p6leng_pikk.webm.480p.vp9.webm",
]

RAW_VIDEO = "/tmp/open_adas_source.webm"
VIDEOS_DIR = "data/videos"
FPS = 30
WIDTH, HEIGHT = 1280, 720

# sim_list.txt → (video_path, sim_data_path, frames_needed)
SCENARIOS = [
    ("data/videos/collision_1.mp4",          "data/sim_data/collision_1.txt",          1164),
    ("data/videos/collision_2.mp4",          "data/sim_data/collision_2.txt",          1175),
    ("data/videos/max_speed_60.mp4",         "data/sim_data/max_speed_60.txt",          589),
    ("data/videos/lane_departure_warning_1.mp4", "data/sim_data/LDW_1.txt",             948),
    ("data/videos/sl_60_collision.mp4",      "data/sim_data/sl_60_collision.txt",       589),
    ("data/videos/camera_calib_video.mp4",   "data/sim_data/camera_calib_video.txt",   3001),
    ("data/videos/over_speed.mp4",           "data/sim_data/over_speed.txt",            868),
]

# ── Helpers ───────────────────────────────────────────────────────────────────

def run(cmd, check=True):
    print(f"  $ {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if check and result.returncode != 0:
        print(f"  STDERR: {result.stderr[-500:]}")
        raise RuntimeError(f"Command failed: {' '.join(cmd)}")
    return result

def video_duration(path):
    """Return duration in seconds using ffprobe."""
    r = run(["ffprobe", "-v", "error", "-show_entries", "format=duration",
             "-of", "default=noprint_wrappers=1:nokey=1", path])
    return float(r.stdout.strip())

def download_source():
    if os.path.exists(RAW_VIDEO) and os.path.getsize(RAW_VIDEO) > 1_000_000:
        print(f"[*] Source video already exists: {RAW_VIDEO}")
        return
    print("[*] Downloading source driving video from Wikimedia Commons...")
    for url in SOURCE_URLS:
        try:
            run(["wget", "-q", "--show-progress", "-O", RAW_VIDEO, url])
            if os.path.exists(RAW_VIDEO) and os.path.getsize(RAW_VIDEO) > 100_000:
                print(f"[+] Downloaded OK ({os.path.getsize(RAW_VIDEO)//1024} KB)")
                return
        except Exception as e:
            print(f"  [!] Failed: {e}, trying next URL...")
    raise RuntimeError("All download URLs failed.")

def make_scenario_video(output_path, frames_needed):
    """
    Build a video of exactly `frames_needed` frames at 30fps from the source.
    If source is shorter, loop it. Scale to 1280x720.
    """
    duration_needed = frames_needed / FPS
    src_dur = video_duration(RAW_VIDEO)
    
    print(f"  → {output_path}: need {frames_needed} frames ({duration_needed:.1f}s), "
          f"source is {src_dur:.1f}s")

    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    # Build ffmpeg command
    # -stream_loop -1 to loop if needed, -t to trim to exact duration
    cmd = [
        "ffmpeg", "-y",
        "-stream_loop", "-1",          # loop source indefinitely
        "-i", RAW_VIDEO,
        "-t", str(duration_needed),    # trim to exact duration
        "-vf", f"scale={WIDTH}:{HEIGHT}:flags=lanczos,fps={FPS}",
        "-an",                          # no audio
        "-c:v", "libx264",
        "-preset", "fast",
        "-crf", "23",
        "-pix_fmt", "yuv420p",
        output_path,
    ]
    run(cmd)
    print(f"  [+] Created: {output_path}")

# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    print("=" * 60)
    print("  OpenADAS Simulation Video Preparation")
    print("=" * 60)

    # Step 1: download source footage
    download_source()

    # Step 2: generate each scenario video
    print("\n[*] Generating scenario videos...")
    for video_path, data_path, frames in SCENARIOS:
        print(f"\n[Scenario] {video_path}")
        try:
            make_scenario_video(video_path, frames)
        except Exception as e:
            print(f"  [!] ERROR: {e}")

    # Step 3: update sim_list.txt to point to correct filenames
    sim_list_content = f"""{len(SCENARIOS)}
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
        f.write(sim_list_content)
    print(f"\n[+] Updated data/sim_list.txt")

    print("\n" + "=" * 60)
    print("  All done! Videos are ready in data/videos/")
    print("  Run the simulation with:")
    print("    ./build/bin/OpenADAS --input_source simulation")
    print("  Or directly:")
    print("    ./build/bin/OpenADAS --input_source simulation \\")
    print("       --input_video_path data/videos/collision_1.mp4 \\")
    print("       --input_data_path data/sim_data/collision_1.txt")
    print("=" * 60)

if __name__ == "__main__":
    main()
