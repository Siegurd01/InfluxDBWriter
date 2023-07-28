# InfluxDBWriter
Yet another InfluxDB C++ library that allows to write ~1.2MSPS using curl.

The batch size is limited to CURL_MAX_INPUT_LENGTH (8 MB)
