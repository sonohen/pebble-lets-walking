"""Independent UI fixture: only initial goal differs; production files unchanged."""
from pathlib import Path
import json, shutil, subprocess, time, tempfile, hashlib
CLI = "/Users/yusuke/.local/bin/pebble"
root = Path.cwd()
fixture = Path(tempfile.mkdtemp(prefix="letswalking-ui-fixture-"))
for name in ["src", "resources"]: shutil.copytree(root/name, fixture/name)
shutil.copy(root/"wscript", fixture/"wscript")
package = json.loads((root/"package.json").read_text())
package["pebble"]["uuid"] = "8cfe5761-2fd7-411a-a829-b0e0db1aa73f"
package["pebble"]["displayName"] = "Walking UI Fixture"
(fixture/"package.json").write_text(json.dumps(package))
main = fixture/"src/c/main.c"
main.write_text(main.read_text().replace("persist_exists(TARGET_KEY) ? persist_read_int(TARGET_KEY) : 8000", "100000"))
out = root/"artifacts/screenshots/limits-fixture"
out.mkdir(parents=True, exist_ok=True)
(out/"README.txt").write_text("Screen-only fixture, independent UUID, initial target forced to 100000. Production source/package unchanged. Fixture path: " + str(fixture) + "\n")
def run(*args): subprocess.run([CLI, *args], check=True, cwd=fixture)
run("build")
fixture_pbw = next((fixture/"build").glob("*.pbw"))
(out/"fixture-pbw.sha256").write_text(hashlib.sha256(fixture_pbw.read_bytes()).hexdigest()+"\n")
for platform in ["diorite", "chalk", "emery"]:
    run("install", "--emulator", platform)
    time.sleep(1.5)
    run("screenshot", "--emulator", platform, "--no-open", str(out/(platform+"-goal100000.png")))
    run("emu-button", "--emulator", platform, "click", "select")
    time.sleep(1.5)
    run("screenshot", "--emulator", platform, "--no-open", str(out/(platform+"-settings100000.png")))
