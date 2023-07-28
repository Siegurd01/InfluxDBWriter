# InfluxDBWriter
Yet another InfluxDB C++ library that allows to write ~1.2MSPS using curl.

Tested on 
```
# uname -a
Linux raspberrypi400 6.1.21-v8+ #1642 SMP PREEMPT Mon Apr  3 17:24:16 BST 2023 aarch64 GNU/Linux
```
InfluxDB V2.7 was installed on Windows 10 (SSD)

The batch size is limited to CURL_MAX_INPUT_LENGTH (8 MB). You get a segmentation fault if you exceed the size of the dataPoint array so be careful.

To read debug messages compile with:
```
g++ -std=c++11 -o Test_write_speed Test_write_speed.cpp InfluxDBWriter.cpp -lcurl -lrt -DDEBUG_MODE=1
```
Or:
```
g++ -std=c++11 -o Test_write_speed Test_write_speed.cpp InfluxDBWriter.cpp -lcurl -lrt -DDEBUG_MODE=0
```
to disable debug messages.

Terminal output (debug option is 1):
```
#./Test_write_speed
InfluxDB is Healthy and ready for queries and writes.
Data written successfully.
Output:
batchSize: 27450
Data written in, ns: 244415658
Speed is: 1.2354e+06 Values per second
InfluxDB is Healthy and ready for queries and writes.
```
# TODO
InfluxDB queries
