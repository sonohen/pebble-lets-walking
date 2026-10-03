"""Capture current app UI on five Pebble form factors; runs SDK outside sandbox."""
from pathlib import Path
import subprocess, time
CLI = "/Users/yusuke/.local/bin/pebble"
out = Path("artifacts/screenshots/v3")
out.mkdir(parents=True, exist_ok=True)
def run(*args):
    subprocess.run([CLI, *args], check=True)
def capture(platform, state):
    time.sleep(1.5)
    run("screenshot", "--emulator", platform, "--no-open", str(out / (platform + "-" + state + ".png")))
def button(platform, name):
    run("emu-button", "--emulator", platform, "click", name)
for platform in ["diorite", "basalt", "chalk", "emery", "gabbro"]:
    run("install", "--emulator", platform)
    capture(platform, "ready")
    button(platform, "select")
    capture(platform, "settings")
    button(platform, "back")
    button(platform, "up")
    capture(platform, "walking")
    button(platform, "select")
    capture(platform, "paused")
    button(platform, "down")
    capture(platform, "finished")
