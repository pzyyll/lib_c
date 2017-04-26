#include  	"app_transaction_api.h"

using namespace snslib;

CTransactionAPI::CTransactionAPI(unsigned int uiUin, unsigned short ushType, unsigned int uiID) {
	memset(&m_stTransactionHeader, 0, sizeof(m_stTransactionHeader));
	m_stTransactionHeader.uiUin = uiUin;
	m_stTransactionHeader.uiTransactionID = uiID;
	m_stTransactionHeader.ushType = ushType;
	m_iBufLen = 0;
	m_ushStepID = 1;
	m_ushLevel = 1;
	memset(m_szBuf, 0, sizeof(m_szBuf));
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
	memcpy(m_szBuf, &m_stTransactionHeader, sizeof(m_stTransactionHeader));
	m_iBufLen += sizeof(m_stTransactionHeader);
}

CTransactionAPI::CTransactionAPI(unsigned int uiUin) {
	memset(&m_stTransactionHeader, 0, sizeof(m_stTransactionHeader));
	m_stTransactionHeader.uiUin = uiUin;
	m_stTransactionHeader.uiTransactionID = 0;
	m_stTransactionHeader.ushType = TRANSACTION_TYPE_ADD;
	m_iBufLen = 0;
	m_ushStepID = 1;
	m_ushLevel = 1;
	memset(m_szBuf, 0, sizeof(m_szBuf));
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
	memcpy(m_szBuf, &m_stTransactionHeader, sizeof(m_stTransactionHeader));
	m_iBufLen += sizeof(m_stTransactionHeader);
}

CTransactionAPI::~CTransactionAPI() {
}

int CTransactionAPI::AddStep(unsigned int uiUin, unsigned short ushType, unsigned short ushLevel, unsigned short ushAppID, unsigned short ushSvrID,
		unsigned short ushCmd, char * pszData, unsigned int uiDataLen) {
	if ((ushType != STEP_TYPE_APP) && (ushType != STEP_TYPE_LOGIC))
		return ERROR_PARAM;
	if ((ushLevel != STEP_LEVEL_AUTO) && (ushLevel != STEP_LEVEL_HOLD))
		return ERROR_PARAM;
	if ((m_stTransactionHeader.ushType != TRANSACTION_TYPE_ADD) && (m_stTransactionHeader.ushType != TRANSACTION_TYPE_MODIFY))
		return ERROR_PARAM;

	TransactionHeader * pTransactionHeader = (TransactionHeader *) m_szBuf;
	StepHeader stStepHeader;
	stStepHeader.uiUin = uiUin;
	stStepHeader.ushID = m_ushStepID++;
	stStepHeader.ushType = ushType;
	stStepHeader.ushLevel = m_ushLevel;
	stStepHeader.ushAppID = ushAppID;
	stStepHeader.ushSvrID = ushSvrID;
	stStepHeader.ushCmd = ushCmd;
	stStepHeader.uiParmLen = uiDataLen;
	stStepHeader.ushProcessFlag = STEP_NOT_PROCESS;
	stStepHeader.iRet = 0;

	memcpy(m_szBuf + m_iBufLen, &stStepHeader, sizeof(stStepHeader));
	m_iBufLen += sizeof(stStepHeader);

	if ((pszData) && (uiDataLen > 0)) {
		if (uiDataLen > (sizeof(m_szBuf) - m_iBufLen))
			return ERROR_BUFF_LESS;
		memcpy(m_szBuf + m_iBufLen, pszData, uiDataLen);
		m_iBufLen += uiDataLen;
	}

	pTransactionHeader->ushStepNum++;
	if (ushLevel == STEP_LEVEL_AUTO) {
		m_ushLevel++;
	}

	return SUCCESS;
}

int CTransactionAPI::AddStep(unsigned int uiUin, unsigned short ushSvrID, unsigned short ushCmd, const char * pszData, unsigned int uiDataLen ){
	if ((m_stTransactionHeader.ushType != TRANSACTION_TYPE_ADD) && (m_stTransactionHeader.ushType != TRANSACTION_TYPE_MODIFY))
		return ERROR_PARAM;

	TransactionHeader * pTransactionHeader = (TransactionHeader *) m_szBuf;
	StepHeader stStepHeader;
	stStepHeader.uiUin = uiUin;
	stStepHeader.ushID = m_ushStepID++;
	stStepHeader.ushType = STEP_TYPE_APP;
	stStepHeader.ushLevel = m_ushLevel;
	stStepHeader.ushAppID = 0;
	stStepHeader.ushSvrID = ushSvrID;
	stStepHeader.ushCmd = ushCmd;
	stStepHeader.uiParmLen = uiDataLen;
	stStepHeader.ushProcessFlag = STEP_NOT_PROCESS;
	stStepHeader.iRet = 0;

	memcpy(m_szBuf + m_iBufLen, &stStepHeader, sizeof(stStepHeader));
	m_iBufLen += sizeof(stStepHeader);

	if ((pszData) && (uiDataLen > 0)) {
		if (uiDataLen > (sizeof(m_szBuf) - m_iBufLen))
			return ERROR_BUFF_LESS;
		memcpy(m_szBuf + m_iBufLen, pszData, uiDataLen);
		m_iBufLen += uiDataLen;
	}

	pTransactionHeader->ushStepNum++;
	m_ushLevel++;

	return SUCCESS;
}

char * CTransactionAPI::GetBufAddr()
{
	return m_szBuf;
}

unsigned int CTransactionAPI::GetBufLen()
{
	return m_iBufLen;
}
