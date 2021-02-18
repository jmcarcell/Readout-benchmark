To compile run

```
mkdir build
cd build
cmake ../
make
```

To run, from the build dir (output file is irrelevant):
```
./custom-dumpfile-to-text -i inputfile -o outputfile
```

A new file hist.txt will be created

To plot, from the build dir (ROOT has to be installed with python support):
```
python3 read.py
```
