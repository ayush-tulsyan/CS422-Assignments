# Fidgeting-ia32
Pin tool for instrument SPEC 2006 binaries compiled for ia32 ISA

### Using this pintool
1. Download [pintool](pintool.org) and untar the download file.
2. Create a link with name ```PIN\_TOOL``` in the current dir to the pintool dir

### Running the tests
To run the pin tool on SPEC 2006 benchmark suite, we have divided the binaries into two sets. The first one contains 8 binaries with runtime within 8 hours. The other set contains 5 binaries which take around 24 hours each. The details can be found in ```Programming_Assignment.pdf```.

1. Use ```make_spec.sh``` to generate the ```spec_2006``` folder. This contains the binaries from the SPEC 2006
2. Copy the scripts ```run_basic.sh``` or ```run_heavy.sh``` from the ```results/``` dir to ```spec_2006``` dir and execute them from within the new folder
