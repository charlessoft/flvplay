
#include <jsoncpp/json/json.h>
/*#include <json/json.h>*/

#include <boost/asio.hpp>
/*#include <netinet/in.h>*/

#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
/*#include <syslog.h>*/
#include <stdarg.h>
#include "CService.h"
using namespace boost::asio;

extern "C" void FX_OUTPUT_LOG_FUNC(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    FILE* file = fopen("c:/logfile.txt", "a+");
    if (file == NULL) return;
    vfprintf(file, format, argList);
    fprintf(file, "\n");
    fclose(file);
    va_end(argList);

}

int GetSpecialWordsA( const char* lpstrSentence, const char* lpPrefix, const char* lpPostfix, char* lpDest, int nMaxSize )
{
    size_t nLength;
    char *pPos, *pEnd;

    pPos = (char*)strstr(lpstrSentence, lpPrefix);
    if( pPos == NULL )
    {
        //strcpy_s(lpDest, nMaxSize, "");
        strcpy(lpDest,"");
        return 0;
    }

    pPos += strlen(lpPrefix);
    while(*pPos == ' ')
        pPos++;

    pEnd = strstr(pPos, lpPostfix);
    if( pEnd == NULL )
        nLength = strlen(lpstrSentence) - strlen(lpPrefix);
    else 
        nLength = (int)(pEnd-pPos);

    //strncpy_s(lpDest, nMaxSize, pPos,nLength);
    strncpy(lpDest,pPos,nLength);


    return (int)nLength;
}
std::string get_uds_filepath_by_curl(const std::string& host, int port, const std::string& uri, const std::string& getargs)
{

        std::stringstream ss;
        string strurl;
        string strurltmp;
        ss << "http://" << host.c_str() << ":" << port << uri.c_str() << "?" << getargs.c_str(); 
//    	string strurl="http://10.142.51.171:7002/media/media-file-play/0/play?sysCheckNo=3452B1CFFA4080468AF1A64DF7C66255&documentid=090f1b318013f255&versionid=090f1b318013f255_0&type=nginx";
        strurltmp = ss.str();
        int npos = strurltmp.find("HTTP",0);
        strurl = strurltmp.substr(0,npos);
        //strurltmp = strurl.substr(0,strurl.length() - strlen("HTTP/1.1\r\nHost"));
        FX_OUTPUT_LOG_FUNC("url = %s",strurl.c_str());
        Service CService;
        int nRes = CService.HttpRequest("GET",strurl);
        printf("nRes = %d",nRes);
        string strbuf =	CService.m_resp_buffer;
        string strresp = CService.m_resp_header;
        FX_OUTPUT_LOG_FUNC("buf=%s",strbuf.c_str());
        FX_OUTPUT_LOG_FUNC("resp=%s",strresp.c_str());
        return  strbuf;  
}

std::string get_uds_filepath_by_httpget(const std::string& host, int port, const std::string& uri, const std::string& getargs) {
    return get_uds_filepath_by_curl( host, port, uri, getargs);
    std::string filepath;
    //std::string filepath = "{\"success\":true,\"relative_path\":\"/media/spring.flv\"}";
    //std::string filepath = "{\"success\":false,\"error_info\":\"UDS get filepath 错误信息\"}";

    //openlog("flvplay", 0, LOG_USER);
    io_service ioservice;
    ip::tcp::socket sock(ioservice); 
    //syslog(LOG_ERR, "endpoint host: %s port: %d", host.c_str(), port);
    //ip::tcp::endpoint endpoint(ip::address::from_string(host.c_str()), port);

    // -----------------------------------------------
    char szport[16];
    sprintf(szport, "%d", port);
    ip::tcp::resolver resolver(ioservice);
    ip::tcp::resolver::query query(host, std::string(szport));
    boost::system::error_code ec_resolve;
    ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec_resolve);
    ip::tcp::endpoint endpoint = *endpoint_iterator;
    // -----------------------------------------------
FX_OUTPUT_LOG_FUNC("xxx=%s://%d/%s?%s",host.c_str(),port,uri.c_str(),getargs.c_str());
    //ip::address addr = ip::address::from_string("127.0.0.1");
    //ip::tcp::endpoint endpoint(addr, port);

    // -----------------------------------------------

    boost::system::error_code ec;
    sock.connect(endpoint, ec);
    if ( ec ) {
        FX_OUTPUT_LOG_FUNC("socket connect failture");
        std::cout << "Socket connect " << host << " failure!" << std::endl;
    } else {
        //boost::asio::streambuf request; 
        //std::ostream request_stream(&request); 
        //request_stream << "GET " << uri << " HTTP/1.1\r\n"; 
        //request_stream << "User-agent: curl/7.27.0\r\n";
        //request_stream << "Host: " << host << ":" << port << "\r\n"; 
        //request_stream << "Accept: */*\r\n"; 
        //request_stream << "Connection: close\r\n"; 
        //request_stream << "\r\n";

        std::stringstream ss;
        if ( !uri.empty() ){
            ss << "GET " << uri << "?" << getargs << " HTTP/1.1\r\n"; 
        } else {
            ss << "GET " << "?" << getargs << " HTTP/1.1\r\n";
        }
        //ss << "User-agent: curl/7.27.0\r\n";
        ss << "User-agent: flvplay/1.0.0\r\n";
        ss << "Host: " << host << ":" << port << "\r\n"; 
        ss << "Accept: */*\r\n"; 
        ss << "Connection: close\r\n"; 
        ss << "\r\n";

        // Send the request. 
        //boost::asio::write(sock, request); 
        const std::string str = ss.str();
        const char* szRequest = str.c_str(); 
        FX_OUTPUT_LOG_FUNC("szRequest=%s",szRequest);
        size_t nRequest = str.length();
        sock.send(buffer(szRequest, nRequest));

        char szResponse[1024];
        size_t nResponse = sizeof(szResponse);
        size_t nBytes = sock.receive(buffer(szResponse, nResponse), 0);

        if ( nBytes <= nResponse )
            szResponse[nBytes] = 0;
        else
            szResponse[nResponse] = 0;

        FX_OUTPUT_LOG_FUNC("szResponse=%s",szResponse);
        filepath = szResponse;

        //std::fstream f;
        //f.open("/tmp/flvplay.txt", std::ios::binary | std::ios::out | std::ios::trunc);
        //f.write(szResponse, nBytes);
        //f.close();
    }

    sock.close();

    return filepath;
}

/*#include <boost/bind.hpp>*/
//#define HAVE_CONFIG_H
//#include <curlpp/cURLpp.hpp>
//#include <curlpp/Easy.hpp>
//#include <curlpp/Options.hpp>
//#include <curlpp/Exception.hpp>
//#include <curlpp/Infos.hpp>

//struct GetFilePathCallback
//{
    //public:
        //GetFilePathCallback(){
            //mStream = NULL;
            //writeRound = 0;
        //};

    //public:
        //GetFilePathCallback(std::ostream * stream)
            //: mStream(stream) , writeRound(0) {
        //}

        //// Helper Class for reading result from remote host
        //size_t write(curlpp::Easy *handle, char* ptr, size_t size, size_t nmemb) {
            //++writeRound;
            //size_t realsize = size * nmemb;

            //std::cout << "WriteFunction() realsize = " << realsize << std::endl;
            ////curlpp::options::Url url;
            ////handle->getOpt(url);
            ////std::cerr << "write round: " << writeRound << ", url: " << url.getValue() << std::endl;

            //if ( mStream != NULL ){
                //mStream->write(ptr, realsize);
            //}
           
            //return realsize;
        //};

        //size_t headerwrite(curlpp::Easy *handle, char* ptr, size_t size, size_t nmemb) {

            //responseHeaders.clear();

            //size_t realsize = size * nmemb;

            ////std::cout << "[header] " << std::string(ptr) << std::endl;

            //if ( realsize > 0 ){
                //std::string item(ptr);
                //size_t n = item.find(":");
                //if ( n != std::string::npos ){
                    //std::string key = item.substr(0, n);
                    //std::string value;
                    //if ( n < item.length() - 2 )
                        //value = item.substr(n + 2, item.length() - n - 2 - 2);
                    //responseHeaders[key] = value;
                //}
            //}

            //return realsize;
        //};

        //std::string getResponseHeadersString() const
        //{
            //std::stringstream ss;
            //for ( std::map<std::string, std::string>::const_iterator it = responseHeaders.begin() ; it != responseHeaders.end() ; it++ ){
                //std::string key = (*it).first;
                //std::string value = (*it).second;

                //ss << key << " : " << value << std::endl;
            //}
            //return ss.str();
        //}

        //std::string getResponseHeader(const std::string& key) const
        //{
            //std::map<std::string, std::string>::const_iterator it = responseHeaders.find(key);
            //if ( it != responseHeaders.end() )
            //{
                //return (*it).second;
            //} else
                //return "";
        //}

        //// Public member vars
        //std::ostream * mStream;
        //unsigned writeRound;
        //std::map<std::string, std::string> responseHeaders;
//};

//std::string get_uds_filepath_by_httpget(const std::string& url, const std::string& getargs)
//{
    //GetFilePathCallback callbackObject;
    //bool bVerbose = false;
    //std::string filepath;

    //try {
        //curlpp::Cleanup cleaner;
        //curlpp::Easy request;

		//using namespace curlpp::Options;

        //request.setOpt(new Verbose(bVerbose));

        //// ---------- Debug ----------
        //curlpp::Options::BoostWriteFunction *test = new curlpp::options::BoostWriteFunction(
                //boost::bind(&GetFilePathCallback::write, &callbackObject, &request, _1, _2, _3)
                //);
        //request.setOpt(test);
    
        //curlpp::Options::BoostHeaderFunction *headerwriter = new curlpp::options::BoostHeaderFunction(
                //boost::bind(&GetFilePathCallback::headerwrite, &callbackObject, &request, _1, _2, _3)
                //);
        //request.setOpt(headerwriter);

        //// ---------- URL ----------
		//request.setOpt(new Url(url));

        //// ---------- Perform Request ----------
		//request.perform();

        //// ---------- Response ----------
        //int responseCode = curlpp::infos::ResponseCode::get(request);
		//std::cout << std::endl << "-=- Response code -=- " << responseCode << std::endl;

        //std::cout << std::endl << "-=- Response Headers -=-" << std::endl << std::endl
            //<< callbackObject.getResponseHeadersString() 
            //<< std::endl;

        //switch ( responseCode ){
            //// OK
            //case 200:{
            //} break;
            //// Multiple Choices
            //case 300: {
            //} break;
            //// Not Modified
            //case 304: {
            //} break;
        //}; // switch ( responseCode )

    //} catch (curlpp::LogicError & e) {
		//std::cout << e.what() << std::endl;
	//} catch (curlpp::RuntimeError & e) {
		//std::cout << e.what() << std::endl;
	//}

    //return filepath;
/*}*/

extern "C" {

    char* get_uds_filepath(const char* host, int port, const char* uri, const char* getargs, char** filepath){ 
        //std::string jsonstring = std::string("{\"sysCheckNo\":\"") + 
        //std::string(sysCheckNo) +
        //std::string("\",\"documentId\":\"") +
        //std::string(documentId) +
        //std::string("\",\"versionId\":\"") +
        //std::string(versionId) +
        //std::string("\"}");

        char* last = NULL;

        std::string jsonstring = get_uds_filepath_by_httpget(host, port, uri, getargs);

        // ---------- jsoncpp ----------
        Json::Value root;
        Json::Reader reader;

        bool bSuccess;
        std::string relative_path;
        std::string error_info;

        *filepath = NULL;

        bool bOK = reader.parse(jsonstring, root);
        if ( bOK ){
            bSuccess = root.get("success", false).asBool();
            if ( bSuccess ) {
                relative_path = root.get("media_relative_path", "").asString();
                size_t len = relative_path.length();
                *filepath = new char[len];
                memcpy(*filepath, relative_path.c_str(), len);
                last = *filepath + len;
           } else {
                error_info = root.get("error_info", "").asString();
            }
        }
         
        // ---------- json-c ----------
        //struct json_object *root = json_tokener_parse(jsonstring.c_str());
        //if ( root != NULL ) {
        //    struct json_object *objSuccess = json_object_object_get(root, "success");
        //    if ( json_object_get_boolean(objSuccess) ) {
        //        struct json_object *objRelativePath = json_object_object_get(root, "relative_path");
        //        std::string relative_path = json_object_get_string(objRelativePath);
        //        size_t len = relative_path.length();
        //        *filepath = new char[len];
        //        memcpy(*filepath, relative_path.c_str(), len);
        //        last = *filepath + len;
        //    } else {
        //        struct json_object *objErrorInfo = json_object_object_get(root, "error_info");
        //        std::string error_info = json_object_get_string(objErrorInfo);
        //    }
        //}

        return last;
    }


} // extern "C"

