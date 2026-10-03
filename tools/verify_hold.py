"""Real button push/release verification without modified app source."""
from pathlib import Path
import subprocess, time, hashlib, json
CLI = "/Users/yusuke/.local/bin/pebble"
platform = "diorite"
out = Path("artifacts/screenshots/hold-"+time.strftime("%Y%m%d-%H%M%S"))
out.mkdir(parents=True)
def run(*args): subprocess.run([CLI,*args],check=True)
def snap(state):
    time.sleep(.5)
    run("screenshot","--emulator",platform,"--no-open",str(out/(state+".png")))
def button(action,key):run("emu-button","--emulator",platform,action,key)
run("install","--emulator",platform)
button("click","select")
snap("00-before")
button("push","up")
try:
    time.sleep(2)
    snap("01-up-held")
finally:button("release","up")
snap("02-up-released")
time.sleep(2)
snap("03-up-stopped")
button("push","down")
try:
    time.sleep(2)
    snap("04-down-held")
finally:button("release","down")
snap("05-down-released")
time.sleep(2)
snap("06-down-stopped")
button("click","back")
snap("07-back-discarded")
(out/"sha256.json").write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.glob("*.png")},indent=2)+"\n")
print("HOLD_EVIDENCE="+str(out))
