#include "qqgamefeeds_api.h"
#include "json.h"

using namespace snslib;

CQQGameFeedsAPI::CQQGameFeedsAPI()
{
}

CQQGameFeedsAPI::~CQQGameFeedsAPI()
{
}

int CQQGameFeedsAPI::AddFeeds(unsigned int uiUin, unsigned int uiAppID, const char *pszAccessToken, const char *pszApiKey, const char *pszContent)
{
    int Ret = 0;
    char szURL[]="http://api.minigame.qq.com/gameapi_feed";
    char szPostData[10240];
    snprintf(szPostData, sizeof(szPostData), "uin=%u&access_token=%s&apikey=%s&appid=%u&charset=gbk&txt=%s", uiUin, pszAccessToken, pszApiKey, uiAppID, pszContent);
    std::string PostData = szPostData;
    std::string RetContent;

    Ret = SendAndRecv(szURL, PostData, RetContent);

    if (Ret != 0)
    {
        return Ret;
    }

    json_object *pCurJsonObj = NULL;

    pCurJsonObj = json_tokener_parse(RetContent.c_str());
    if (is_error(pCurJsonObj))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to parse json ret content [%s]", RetContent.c_str());
        return -1;
    }

    json_object * pField = NULL;
    pField = json_object_object_get(pCurJsonObj, "result");
    if (is_error(pField))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to get result from json ret content [%s]", RetContent.c_str());
        json_object_put(pCurJsonObj);
        return -1;
    }

    Ret = json_object_get_int(pField);
    json_object_put(pField);

    if (Ret != 0)
    {
        pField = json_object_object_get(pCurJsonObj, "resultstr");
        if (is_error(pField))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to get resultstr from json ret content [%s]", RetContent.c_str());
            json_object_put(pCurJsonObj);
            return -1;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", json_object_get_string(pField));
        json_object_put(pField);
    }

    json_object_put(pCurJsonObj);

    return Ret;
}

size_t CQQGameFeedsAPI::CurlWriterFunc(char *data, int size, int nmemb, std::string *content)
{
    //writer
    size_t sizes = size * nmemb;
    std::string temp;
    temp.assign(data, sizes);
    (*content) += temp;
    return sizes;
}

int CQQGameFeedsAPI::SendAndRecv(const char* pszURL, std::string &PostData, std::string &RetContent)
{
    int Ret = 0;

    m_CURL = curl_easy_init();
    if (m_CURL == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to create CURL connection");
        return -1;
    }

    Ret = SendAndRecvInternal(pszURL, PostData, RetContent);

    curl_easy_cleanup(m_CURL);

    return Ret;
}

int CQQGameFeedsAPI::SendAndRecvInternal(const char* pszURL, std::string &PostData, std::string &RetContent)
{
    CURLcode RetCode;

    RetCode = curl_easy_setopt(m_CURL, CURLOPT_ERRORBUFFER, m_CURLErrBuff);
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to set error buffer [%d]", RetCode);
        return -1;
    }

    /*
    RetCode = curl_easy_setopt(m_CURL, CURLOPT_FOLLOWLOCATION, 1);
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to set redirect option [%s]", m_CURLErrBuff);
        curl_easy_cleanup(m_CURL);
        return -1;
    }
    */

    RetCode = curl_easy_setopt(m_CURL, CURLOPT_WRITEFUNCTION, CurlWriterFunc);
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to set writer [%s]", m_CURLErrBuff);
        return -1;
    }

    RetCode = curl_easy_setopt(m_CURL, CURLOPT_WRITEDATA, &RetContent);
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to set write data [%s]", m_CURLErrBuff);
        return -1;
    }

    RetCode = curl_easy_setopt(m_CURL, CURLOPT_POSTFIELDS, PostData.c_str());
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to set post data [%s]", m_CURLErrBuff);
        return -1;
    }

    RetCode = curl_easy_setopt(m_CURL, CURLOPT_URL, pszURL);
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to set URL [%s]", m_CURLErrBuff);
        return -1;
    }

    RetCode = curl_easy_perform(m_CURL);
    if (RetCode != CURLE_OK)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to get '%s' [%s]", pszURL, m_CURLErrBuff);
        return -1;
    }

    long HttpRetCode = 0;
    RetCode = curl_easy_getinfo(m_CURL, CURLINFO_RESPONSE_CODE, &HttpRetCode);
    if ((RetCode == CURLE_OK) && HttpRetCode == 200)
    {
        double HttpRetLength = 0;
        RetCode = curl_easy_getinfo(m_CURL, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &HttpRetLength);

        return 0;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "URL ret=%lu, error=%s", HttpRetCode, m_CURLErrBuff);
    }

    return -1;
}

