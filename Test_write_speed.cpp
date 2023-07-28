#include "InfluxDBWriter.h"
#include <unistd.h>

#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

const char* ipAddress = "192.168.x.x";
const int portNumber = 8086;
const char* organization = "Some_Org";
const char* bucket = "Some_bucket_name";
const char* precision = "ns";
const char* measurement = "Some_measurement_name";
const char* tags = "Some_tag=Some_tag1";
const char* influxToken = "1111111111111111111111111111111111111111111111111111111111111111111111111111111!==";

const uint32_t maxDataPointSize = 300; // change it to your line data size!!!!!!
const uint32_t maxBatchSize = 8 * 1024 * 1024; // 8 MB (CURL_MAX_INPUT_LENGTH) 
const uint32_t batchSize = maxBatchSize/maxDataPointSize-512; // maximum batchSize -512 to account for additional data size (headers, etc.)
//const uint32_t batchSize = 2;

const char* val_names[] = {"Sensor_1", "Sensor_2", "Sensor_3", "Sensor_4", "Sensor_5", "Sensor_6", "Sensor_7", "Sensor_8", "Sensor_9", "Sensor_10", "Sensor_11"};
const size_t val_size = sizeof(val_names) / sizeof(val_names[0]);
float val_values[val_size] = {21.0, 35.9, 0.0, 35.9, 35.9, 35.9, 35.9, 35.9, 35.9, 35.9, 35.9};
const char decimal = 5; 

timespec time1, time2;

InfluxDBWriter writer(ipAddress, portNumber, organization, bucket, precision,
                      measurement, tags, influxToken, maxDataPointSize, maxBatchSize,
                      val_names, val_size, decimal, batchSize);

timespec diff(timespec start, timespec end) {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

float x = 0;

int main() {

    for (int i = 0; i < batchSize; ++i) {
       
        clock_gettime(CLOCK_REALTIME, &time1);
		val_values[0]=(float)Sin13((double)x);
        writer.appendToBuffer(time1, val_values);
		x=x+(float)0.0005;
		//usleep(1000);
    }

    if (!writer.checkHealth()) {
        std::cerr << "InfluxDB is not healthy. Aborting data writing process." << std::endl;
        return 1;
    }
	
	#if DEBUG_MODE
		clock_gettime(CLOCK_REALTIME, &time1);
	#endif
    if (!writer.writeToInfluxDB()) {
        std::cerr << "Failed to write data to InfluxDB." << std::endl;
        return 1;
    }
	#if DEBUG_MODE
		clock_gettime(CLOCK_REALTIME, &time2);
	    std::cout<< "batchSize: " << batchSize << std::endl;
		std::cout<< "Data written in, ns: " << diff(time1,time2).tv_nsec << std::endl;
		std::cout<< "Speed is: " << (double)(1000000000.0/diff(time1,time2).tv_nsec)*(double)(batchSize*val_size) << " Values per second" << std::endl;
	#endif

    if (!writer.checkHealth()) {
        std::cerr << "InfluxDB is not healthy. Aborting data writing process." << std::endl;
        return 1;
    }
    return 0;
}
