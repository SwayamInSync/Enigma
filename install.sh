python -m pip uninstall -y enigma
python -m pip install . -v --no-build-isolation -Cbuilddir=build -C'compile-args=-v' -Csetup-args="-Dbuildtype=debug"
