// InfluxDBWriter.h

#ifndef INFLUXDB_WRITER_H
#define INFLUXDB_WRITER_H

#include <string>
#include <iostream>

class InfluxDBWriter {
public:
    InfluxDBWriter(const char* ipAddress, int portNumber, const char* organization,
                   const char* bucket, const char* precision, const char* measurement,
                   const char* tags, const char* influxToken, size_t maxDataPointSize,
                   size_t maxBatchSize, const char* val_names[], 
                   size_t val_size, const char decimal, const uint32_t batchSize);

    bool writeToInfluxDB();
    bool checkHealth();
    bool appendToBuffer(const timespec& time1, const float* val_values);

private:
    const char* ipAddress;
    int portNumber;
    const char* organization;
    const char* bucket;
    const char* precision;
    const char* measurement;
    const char* tags;
    const char* influxToken;
    size_t maxDataPointSize;
    size_t maxBatchSize;
    const char** val_names;
    const float* val_values;
    size_t val_size;
    const char decimal;
	const uint32_t batchSize;
	
    char* influxUrl;
    char* influxUrl_health;
    char* dataPoint;
    size_t dataSize;

};

#endif // INFLUXDB_WRITER_H
