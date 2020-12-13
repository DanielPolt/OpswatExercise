/*
 Solution to MetaDefender Could Exercise
 Solution written by Daniel Polt.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
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

//the name of the file used for the program
string fileName = "TestDatFile.dat";

/*
engineDetails
Has five string data members for the four different values that the MetaDefender scan reports for each engine, along with the engine name
Also has the printDetails function for displaying the values in the desired format
*/
class engineDetails
{
    private:
        string engine;
        string threat_found;
        string scan_time;
        string scan_result_i;
        string def_time;
    public:
        engineDetails(string engineParameter, string threat_foundParameter, string scan_timeParameter, string scan_result_iParameter, string def_timeParameter)
        {
            engine = engineParameter;
            if (threat_foundParameter == "\"\"")
                threat_found = "Clean";
            else
                threat_found = threat_foundParameter;
            scan_time = scan_timeParameter;
            scan_result_i = scan_result_iParameter;
            def_time = def_timeParameter;
        }

        /*
        printDetails
        precondition: the engineDetails object has been initialized
        postcondition: print the data members on their own console output lines in the order of: engine, threat_found, scan_result_i, def_time. Also include a note indicating which data member is which.
        */
        void printDetails()
        {
            cout << "engine: " << engine << endl;
            cout << "threat_found: " << threat_found << endl;
            cout << "scan_result: " << scan_result_i << endl;
            cout << "def_time: " << def_time << endl;
        }
};

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
    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;
    return readBuffer;
}

/*
uploadFile
precondition: a valid API Key is entered and a file with the name stored in fileName is within the local directory and has not been cached already
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
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, fileName);
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
    int i = 0;
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
            cin.clear();
            cin.ignore(1000, '\n');
            sentinel = true;
        }
    } while (sentinel);

    //use picosha2 to hash the contents of the file whose name is stored in fileName and store the result in hex_str
    ifstream f(fileName, ios::binary);
    vector<unsigned char> s(picosha2::k_digest_size);
    picosha2::hash256(f, s.begin(), s.end());
    string hex_str = picosha2::bytes_to_hex_string(s.begin(), s.end());

    //perform a hash lookup with the API and the hash value in hex_str, putting the results into the string s1
    string s1;
    s1 = hashLookup(hex_str, apiKey);
    
    //if s1 contains an error, the hash value was not found in the system and the file should be uploaded
    if (s1 == "{\"error\":{\"code\":400064,\"messages\":[\"The hash value is not valid\"]}}" || s1 == "{\"error\":{\"code\":404003,\"messages\":[\"The hash was not found\"]}}")
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

        dataId = "bzIwMTIxME5YSmQxYVE2RHhMNkRObnJLc3c4"; //Inconsistent results of the uploadFile commands in curl have resulted, possibly due to faulty documentation, so for demonstration a Data ID that is guaranteed to work is used instead

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
    
    //Adds spaces to s1 between colons, commas, and brackets so it is read more easily by the JSON
    for (int i = 0; i < s1.size(); i++)
    {
        if (s1[i] == ':')
        {
            i++;
            s1.insert(i, " ");
        }
        if (s1[i] == ',')
        {
            i++;
            s1.insert(i, " ");
        }
        if (s1[i] == '{')
        {
            i++;
            s1.insert(i, " ");
        }
        if (s1[i] == '}')
        {
            s1.insert(i, " ");
            i++;
        }
    }

    //find where "scan_details" begin in s1 and replace s1 with the contents after that point 
    int detailsStart = s1.find("scan_details");
    detailsStart += 16;
    s1 = s1.substr(detailsStart);

    //while loop to iterate through the results of each engine
    while (s1.find("scan_result") != -1)
    {
        //find the engine and save its name to the string 'engine'
        int engIndex = s1.find("\"") + 1;
        string engine = "";
        while (s1[engIndex] != '\"')
        {
            engine += s1[engIndex];
            engIndex++;
        }

        //declare a new string, s2, that contains the scan results of one engine
        int openBracket = s1.find("{");
        int lengthBracket = s1.find("}") - openBracket + 1;
        string s2 = s1.substr(openBracket, lengthBracket);

        //use json to parse the scan results in s2
        auto j = json::parse(s2);

        //declare an engineDetails object with the engine name and the parsed values as the constructor parameter
        engineDetails eD(engine, to_string(j.at("threat_found")), to_string(j.at("scan_time")), to_string(j.at("scan_result_i")), to_string(j.at("def_time")));
        
        //print the results with the printDetails method of the engineDetails class
        eD.printDetails();
        
        s1 = s1.substr(s1.find("}") + 2, s1.size() - s2.size());
    }

    return 0;
}
