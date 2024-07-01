import sys
import os

def create_version_file(version):
    with open(os.path.join(os.path.dirname(__file__), '..', 'src', 'OC_version.h'), 'w') as f:
        f.write(f'// NOTE: DO NOT INCLUDE DIRECTLY, USE OC::Strings::VERSION\n"{version}"\n')

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Please specify version string")
        sys.exit(1)
    create_version_file(sys.argv[1])