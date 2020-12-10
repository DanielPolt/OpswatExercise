//

#include <iostream>
#include <sstream>
#include <cctype>
#include <string>
#include<cstdio>
#include<cstring>
#include<iostream>
#include <vector>
#include <curl\curl.h>

//hash:  4427d515503057624e16f41b5202dca2a03b6192328bbf49ff7db038ff8fddaf
//temp hash: 0CF1EE1DF35CF98B53FF79CF3D6D5091654FBDE678041EDA21180557A07B2C79
//data ID:  bzIwMTIwOXQ2a0Vyb1pJdGlEMmZvaWI5dXp5

#define CURL_STATICLIB

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static string hashLookup()
{
    CURLcode res;
    CURL* hnd;
    struct curl_slist* slist1;
    string readBuffer;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "apikey: 99a5d4cb19d65809b8afda69ca4410a7");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.metadefender.com/v4/hash/0CF1EE1DF35CF98B53FF79CF3D6D5091654FBDE678041EDA21180557A07B2C79");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.55.1");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;

    return readBuffer;
}

static string scanFile()
{
    CURLcode ret;
    CURL* hnd;
    struct curl_slist* slist1;
    string readBuffer;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "apikey: 99a5d4cb19d65809b8afda69ca4410a7");
    slist1 = curl_slist_append(slist1, "content-type: application/octet-stream");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.metadefender.com/v4/file");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "TestDatFile.dat");
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)15);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.55.1");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);

    ret = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;

    return readBuffer;
}

static string scanDataId(string urlParameter)
{
    CURL* hnd;
    CURLcode res;
    string readBuffer;
    const char* url = urlParameter.c_str();

    struct curl_slist* slist1;
    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "apikey: 99a5d4cb19d65809b8afda69ca4410a7");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.55.1");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;

    return readBuffer;
}



int main(int argc, char* argv[])
{
    



    
    
    string s1 = hashLookup();
    if (s1 == "{\"error\":{\"code\":400064,\"messages\":[\"The hash value is not valid\"]}}")
    {
        string s2 = scanFile();
        int dataStart = s2.find("data_id") + 10;
        int dataLength = s2.find("\",\"status") - dataStart;
        //string dataId = s2.substr(dataStart, dataLength);
        string dataId = "bzIwMTIwOXQ2a0Vyb1pJdGlEMmZvaWI5dXp5";
        string url = "https://api.metadefender.com/v4/file/" + dataId;
        s1 = scanDataId(url);
    }
    
    int detailsStart = s1.find("scan_details");
    detailsStart += 15;
    string reduced = s1.substr(detailsStart);

    while (reduced.find("scan_result") != -1)
    {
        int engIndex = reduced.find("\"") + 1;
        string engine = "";
        while (reduced[engIndex] != '\"')
        {
            engine += reduced[engIndex];
            engIndex++;
        }
        int openBracket = reduced.find("{");
        int lengthBracket = reduced.find("}") - openBracket + 1;
        cout << "engine: " << engine << endl;
        string engDetails = reduced.substr(openBracket, lengthBracket);

        string threatFound;
        int threatStart = engDetails.find("threat_found") + 14;
        int threatLength = engDetails.find(",\"scan_time") - threatStart;
        threatFound = engDetails.substr(threatStart, threatLength);
        if (threatFound == "\"\"")
            threatFound = "Clean";
        cout << "threat_found: " << threatFound << endl;

        string scanResult;
        int resultStart = engDetails.find("scan_result") + 15;
        int resultLength = engDetails.find(",\"def_time") - resultStart;
        scanResult = engDetails.substr(resultStart, resultLength);
        cout << "scan_result: " << scanResult << endl;

        string defTime;
        int timeStart = engDetails.find("def_time") + 11;
        int timeLength = engDetails.find("\"}") - timeStart;
        defTime = engDetails.substr(timeStart, timeLength);
        cout << "def_time: " << defTime << endl;

        reduced = reduced.substr(reduced.find("}") + 2, reduced.size() - engDetails.size());
    }

    return 0;
}
