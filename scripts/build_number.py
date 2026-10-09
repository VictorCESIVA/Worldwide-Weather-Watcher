Import("env")
from pathlib import Path
p = Path(env.subst("$BUILD_DIR")) / "numero-build.txt"
p.parent.mkdir(parents=True, exist_ok=True)
number = int(p.read_text()) + 1 if p.exists() else 1
p.write_text(str(number))
env.Append(CPPDEFINES=[("BUILD_NUMBER", number)])
