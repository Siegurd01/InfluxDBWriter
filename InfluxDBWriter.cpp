// InfluxDBWriter.cpp

#include "InfluxDBWriter.h"
#include <curl/curl.h>
#include <cstring>
#include <iostream>
#include <ctime>

#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif


// Function to send POST requests using cURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

InfluxDBWriter::InfluxDBWriter(const char* ipAddress, int portNumber, const char* organization,
                               const char* bucket, const char* precision, const char* measurement,
                               const char* tags, const char* influxToken, size_t maxDataPointSize,
                               size_t maxBatchSize, const char* val_names[],
                               size_t val_size, const char decimal, const uint32_t batchSize)
    : ipAddress(ipAddress), portNumber(portNumber), organization(organization), bucket(bucket),
      precision(precision), measurement(measurement), tags(tags), influxToken(influxToken),
      maxDataPointSize(maxDataPointSize), maxBatchSize(maxBatchSize), val_names(val_names),
      val_size(val_size), decimal(decimal), batchSize(batchSize) {
    
	influxUrl = new char[256];
    snprintf(influxUrl, 256, "http://%s:%d/api/v2/write?org=%s&bucket=%s&precision=%s",
             ipAddress, portNumber, organization, bucket, precision);

    influxUrl_health = new char[256];
    snprintf(influxUrl_health, 256, "http://%s:%d/health", ipAddress, portNumber);

    dataPoint = new char[maxDataPointSize * batchSize];
    dataSize = 0;
}

bool InfluxDBWriter::appendToBuffer(const timespec& time1,const float* val_values) {
    // Convert the timestamp to InfluxDB Line Protocol format (nanosecond precision)
    char timestamp[64];
    snprintf(timestamp, sizeof(timestamp), "%ld", time1.tv_sec * 1000000000L + time1.tv_nsec);

    // Create the data point with measurement, fields, tags, and the timestamp
    char dataPointBuffer[maxDataPointSize];
    int dataPointSize = snprintf(dataPointBuffer, maxDataPointSize, "%s,%s ", measurement, tags);

    // Add parameter names and values dynamically
    for (size_t j = 0; j < val_size; ++j) {
        dataPointSize += snprintf(dataPointBuffer + dataPointSize, maxDataPointSize - dataPointSize,
                                  "%s=%.*f,", val_names[j], decimal, val_values[j]);
    }

    // Remove the last comma and add the timestamp
    dataPointBuffer[dataPointSize - 1] = ' ';
    dataPointSize += snprintf(dataPointBuffer + dataPointSize, maxDataPointSize - dataPointSize, "%s\n", timestamp);

    if (dataSize + dataPointSize < maxDataPointSize * batchSize) {
        // Append the data point to the batch buffer
        memcpy(dataPoint + dataSize, dataPointBuffer, dataPointSize);
        dataSize += dataPointSize;
        return true;
    } else {
        std::cerr << "Data points exceeded batch size. Skipping the remaining data points." << std::endl;
        return false;
    }
}

bool InfluxDBWriter::writeToInfluxDB() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL." << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    char authHeader[512];
    snprintf(authHeader, sizeof(authHeader), "Authorization: Token %s", influxToken);
    headers = curl_slist_append(headers, authHeader);
    headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");
    headers = curl_slist_append(headers, "Accept: application/json");

    CURLcode res;
    std::string output;

    curl_easy_setopt(curl, CURLOPT_URL, influxUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dataSize);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataPoint);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to write data: " << curl_easy_strerror(res) << std::endl;
    } else {
        #if DEBUG_MODE
			std::cout << "Data written successfully." << std::endl;
			std::cout << "Output: " << output << std::endl;
		#endif
        // Optionally, handle the response from the server (output contains the server response).
    }
	dataSize = 0;
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return true;
}

bool InfluxDBWriter::checkHealth() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL." << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");

    CURLcode res;
    std::string output;

    curl_easy_setopt(curl, CURLOPT_URL, influxUrl_health);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode == 200) {
			#if DEBUG_MODE
				std::cout << "InfluxDB is Healthy and ready for queries and writes." << std::endl;
			#endif
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return true;
        } else {
            std::cerr << "InfluxDB is Unhealthy." << std::endl;
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return false;
        }
    } else {
        std::cerr << "Failed to check InfluxDB health: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return false;
    }
}

