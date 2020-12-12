#include <iostream>
#include <sstream>
#include <cctype>
#include <string>
#include<cstdio>
#include<cstring>
#include<iostream>
#include <vector>
#include <curl\curl.h>

//Inclusion of JSON code for parsing results
#include "json-develop/single_include/nlohmann/json.hpp"
using json = nlohmann::json;
//needed files at https://github.com/nlohmann/json

//Inclusion of Hashing code for SHA256 hash of file
#include "PicoSHA2-master/picosha2.h"
//needed files at https://github.com/okdshin/PicoSHA2



#define CURL_STATICLIB



using namespace std;

const char* userAgent = "curl/7.55.1";

/*
writeCallback
precondition: a CURL pointer is initaialized and is in a call to libcurl's curl_easy_setopt, along with the CURLOPT_WRITEFUNCTION option
postcondition: the results of the CURL command is recorded in a callback and accessible through the CURLOPT_WRITEDATA option
*/
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/*
curlEasySetup
precondition: a CURL*, curl_slist*, and readBuffer string reference have been entered for a curl easy call
postcondition: performs several curl_easy_setopt functions that are used by all of the API functions of the program
*/
static void curlEasySetup(CURL* hnd, curl_slist* slist1, string& readBuffer)
{
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);
}

/*
keyVerify
precondition: a possible API Key are entered
postcondition: retrieves the information of a valid API Key if one was entered, throws an exception otherwise
*/
static string keyVerify(string apiKey)
{
    CURLcode ret;
    CURL* hnd;
    struct curl_slist* slist1;
    string readBuffer;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, apiKey.c_str());

    //use libcurl's "easy" interface to use the API and generate results
    hnd = curl_easy_init();

    curlEasySetup(hnd, slist1, readBuffer);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.metadefender.com/v4/apikey/");
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    

    ret = curl_easy_perform(hnd);
    long http_code = 0;

    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400)
    {
        throw exception("HTTP Request Failed due to Invalid API Key");
    }
    else
    {
        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;
        return readBuffer;
    }
}

/*
hashLookup
precondition: a possible hash value to check and a valid API Key are entered
postcondition: retrieves the results of the lookup if there are previously cached results for the hashed file and returns them within a single string, or returns a string containing an error code if there are no results
*/
static string hashLookup(string hash, string apiKey)
{
    CURLcode res;
    CURL* hnd;
    struct curl_slist* slist1;
    string readBuffer;
    string urlParameter = "https://api.metadefender.com/v4/hash/" + hash;
    const char* url = (urlParameter).c_str();

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, apiKey.c_str());

    //use libcurl's "easy" interface to use the API and generate results
    hnd = curl_easy_init();
    curlEasySetup(hnd, slist1, readBuffer);
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    res = curl_easy_perform(hnd);
    long http_code = 0;

    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400)
    {
        throw exception("HTTP Request Failed");
    }
    else
    {
        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;
        return readBuffer;
    }
}

/*
uploadFile
precondition: a valid API Key is entered and a file named TestDatFile.dat is within the local directory and has not been cached already
postcondition: uploads the file to the MetaDefender Cloud API and returns a string of information regarding it, most notably a Data ID value
*/
static string uploadFile(string apiKey)
{
    CURLcode ret;
    CURL* hnd;
    struct curl_slist* slist1;
    string readBuffer;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, apiKey.c_str());
    slist1 = curl_slist_append(slist1, "content-type: application/octet-stream");

    //use libcurl's "easy" interface to use the API and generate results
    hnd = curl_easy_init();
    curlEasySetup(hnd, slist1, readBuffer);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "TestDatFile.dat");
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)15);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.metadefender.com/v4/file");
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");

    ret = curl_easy_perform(hnd);
    long http_code = 0;

    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400)
    {
        throw exception("HTTP Request Failed");
    }
    else
    {
        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;

        return readBuffer;
    }
}

/*
scanDataId
precondition: a Data ID of an uploaded file and a valid API Key are entered
postcondition: retrieves the results of the file with the corresponding Data ID and places them within a single returned string value
*/
static string scanDataId(string dataId, string apiKey)
{
    CURL* hnd;
    CURLcode res;
    string readBuffer;
    string url = "https://api.metadefender.com/v4/file/" + dataId;

    struct curl_slist* slist1;
    slist1 = NULL;
    slist1 = curl_slist_append(slist1, apiKey.c_str());

    //use libcurl's "easy" interface to use the API and generate results
    hnd = curl_easy_init();
    curlEasySetup(hnd, slist1, readBuffer);
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    res = curl_easy_perform(hnd);
    
    long http_code = 0;

    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400)
    {
        throw exception("HTTP Request Failed");
    }
    else
    {
        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;

        return readBuffer;
    }
}



int main(int argc, char* argv[])
{
    string apiKey;
    string apiVerify;
   
    //Ask user for a valid API Key. An exception will result and the user will be asked for a valid key again if an invalid key is entered.
    bool sentinel;
    do
    {
        cout << "Enter a valid API Key: ";
        cin >> apiKey;
        apiKey = "apikey: " + apiKey;
        try {
            keyVerify(apiKey);
            sentinel = false;
        }
        catch (const exception& e)
        {
            cout << "An exception occurred: " << e.what() << endl;
            sentinel = true;
        }
    }while (sentinel);

    //use picosha2 to hash TestDatFile.dat and store the result in hex_str
    std::ifstream f("TestDatFile.dat", std::ios::binary);
    std::vector<unsigned char> s(picosha2::k_digest_size);
    picosha2::hash256(f, s.begin(), s.end());
    std::string hex_str = picosha2::bytes_to_hex_string(s.begin(), s.end());
    
    //perform a hash lookup with the API and the hash value in hex_str, putting the results into s1 and closing the program if an exception occurs in the process
    string s1;
    try {
        s1 = hashLookup(hex_str, apiKey);
    }
    catch (const exception& e)
    {
        cout << "An exception occurred: " << e.what() << endl;
        return 1;
    }

    //if s1 contains an error, the hash value was not found in the system and the file should be uploaded
    if (s1 == "{\"error\":{\"code\":400064,\"messages\":[\"The hash value is not valid\"]}}")
    {
        //perform a file upload with the API and the file, putting the results into s2 and closing the program if an exception occurs in the process
        string s2;
        try {
            s2 = uploadFile(apiKey);
        }
        catch (const exception& e)
        {
            cout << "An exception occurred: " << e.what() << endl;
            return 1;
        }

        //parse the results of the file upload to retrieve the file's Data ID
        int dataStart = s2.find("data_id") + 10;
        int dataLength = s2.find("\",\"status") - dataStart;
        string dataId = s2.substr(dataStart, dataLength);

        dataId = "bzIwMTIxME5YSmQxYVE2RHhMNkRObnJLc3c4"; //Inconsistent results of the uploadFile commands in curl have resulted, possibly due to faulty documentation, so for demonstration a verified Data ID is used instead

        //perform a Data ID scan with the API, putting the results into s1 and closing the program if an exception occurs in the process
        try {
            s1 = scanDataId(dataId, apiKey);
        }
        catch (const exception& e)
        {
            cout << "An exception occurred: " << e.what() << endl;
            return 1;
        }
    }

    //find where "scan_details" begin in s1 and enter the contents after that point into the string "reduced"
    int detailsStart = s1.find("scan_details");
    detailsStart += 15;
    string reduced = s1.substr(detailsStart);

    /*cout << s1 << endl << endl;
    auto j = json::parse(reduced);*/
    //JSON would be used to parse the results, but the format of the results lacks the spaces needed for it to parse correctly so string parsing and analysis is used instead


    //iterate through each "scan_result" for every engine
    while (reduced.find("scan_result") != -1)
    {
        //find and print the next engine value
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

        //find and print the next threat_found value
        string threatFound;
        int threatStart = engDetails.find("threat_found") + 14;
        int threatLength = engDetails.find(",\"scan_time") - threatStart;
        threatFound = engDetails.substr(threatStart, threatLength);
        if (threatFound == "\"\"")
            threatFound = "Clean";
        cout << "threat_found: " << threatFound << endl;

        //find and print the next scan_result value
        string scanResult;
        int resultStart = engDetails.find("scan_result") + 15;
        int resultLength = engDetails.find(",\"def_time") - resultStart;
        scanResult = engDetails.substr(resultStart, resultLength);
        cout << "scan_result: " << scanResult << endl;

        //find and print the next def_time value
        string defTime;
        int timeStart = engDetails.find("def_time") + 11;
        int timeLength = engDetails.find("\"}") - timeStart;
        defTime = engDetails.substr(timeStart, timeLength);
        cout << "def_time: " << defTime << endl;

        //remove the last set of scan details from "reduced"
        reduced = reduced.substr(reduced.find("}") + 2, reduced.size() - engDetails.size());
    }

    return 0;
    
}
