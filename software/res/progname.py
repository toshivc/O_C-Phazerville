Import("env")
import os

def read_last_line(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
        return lines[-1].strip().replace('"', '')

version = read_last_line(os.path.join(env['PROJECT_DIR'], 'src', 'OC_version.h'))

# You might need to adjust this part depending on what oc_build_tag.sh does
git_rev = env.GetProjectOption("revision", "unknown")

extras = ""
env.Append(BUILD_FLAGS=[f'-DOC_BUILD_TAG=\\"{git_rev}\\"'])

build_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = build_flags.get("CPPDEFINES")
for item in defines:
    if isinstance(item, tuple) and item[0] == 'OC_VERSION_EXTRA':
        extras += item[1].strip('"')

if "VOR" in [d[0] if isinstance(d, tuple) else d for d in defines]:
    extras += "+VOR"
if "FLIP_180" in [d[0] if isinstance(d, tuple) else d for d in defines]:
    extras += "_flipped"

env.Replace(PROGNAME=f"o_C-phazerville-{version}{extras}-{git_rev}")