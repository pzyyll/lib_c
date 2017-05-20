#ifndef QQGAMEFEEDS_API_H
#define QQGAMEFEEDS_API_H

#include <curl/curl.h>
#include <string>

namespace snslib
{

class CQQGameFeedsAPI
{
public:
    CQQGameFeedsAPI();
	~CQQGameFeedsAPI();

	int AddFeeds(unsigned int uiUin, unsigned int uiAppID, const char *pszAccessToken, const char *pszApiKey, const char *pszContent);

	const char *GetErrMsg()
	{
	    return m_szErrMsg;
	}

private:
    int SendAndRecv(const char* URL, std::string &PostData, std::string &RetContent);
    int SendAndRecvInternal(const char* URL, std::string &PostData, std::string &RetContent);
	static size_t CurlWriterFunc(char *data, int size, int nmemb, std::string *content);

private:
	CURL *m_CURL;

	char m_szErrMsg[256];
	char m_CURLErrBuff[CURL_ERROR_SIZE];
};

}
#endif


