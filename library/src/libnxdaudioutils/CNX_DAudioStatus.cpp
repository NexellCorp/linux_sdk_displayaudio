#include "CNX_DAudioStatus.h"
#include <NX_IConfig.h>

#define LOG_TAG "[libnxutils]"
#include <NX_Log.h>

#define DAUDIO_CONFIG	"/nexell/daudio/daudio.xml"

struct cbSqlite3ExecArgs
{
	int argc;
	vector<string> argv;
	vector<string> column;
};
static cbSqlite3ExecArgs g_cbArgs;

static int cbSqlite3Exec(void* data, int argc, char** argv, char** column)
{
	g_cbArgs.argv.clear();
	g_cbArgs.column.clear();
	g_cbArgs.argc = argc;

	// to avoid compiler warning.
	(void)data;

	for (int i = 0; i < argc; i++)
	{
		g_cbArgs.column.push_back(column[i]);
		g_cbArgs.argv.push_back(argv[i]);
	}

	return 0;
}

CNX_DAudioStatus::CNX_DAudioStatus(string database/*= DEFAULT_DAUDIO_STATUS_DATABASE*/)
{
	int volume;
	m_pHandle = NULL;

	size_t length = database.length();
	if (length == 0)
	{
		return;
	}

	m_DataBasePath = new char[length];

	m_DataBasePath = database;

	OpenDataBase();

	CreateTable();

	if (!IsExistByID(0))
	{
		InsertTuple();
	}

	m_CardNumber = "default";
	m_iMaxVolume = 0;
	NX_IConfig *pConfig = GetConfigHandle();
	if (0 == pConfig->Open(DAUDIO_CONFIG))
	{
		char *pBuf = NULL;
		if (0 == pConfig->Read("master_volume", &pBuf))
		{
			m_SoundCard = string(pBuf);
		}

		if (0 == pConfig->Read("max_volume", &pBuf))
		{
			m_iMaxVolume = atoi(pBuf);
		}

		if (0 == pConfig->Read("card_number", &pBuf))
		{
			m_CardNumber = string(pBuf);
		}
	}
	delete pConfig;

	if (0 != pthread_create(&m_hInitThread, NULL, CNX_DAudioStatus::ThreadStub, (void *)this))
	{
		NXLOGI("pthread_create() failed\n");
	}
}

CNX_DAudioStatus::~CNX_DAudioStatus()
{
	if (IsOpenedDataBase())
		CloseDataBase();
}

int32_t CNX_DAudioStatus::OpenDataBase()
{
	int32_t ret = 1;

	if (!m_pHandle)
	{
		ret = (int32_t)(SQLITE_OK == sqlite3_open(m_DataBasePath.c_str(), &m_pHandle));
	}

	return ret;
}

int32_t CNX_DAudioStatus::CloseDataBase()
{
	int32_t ret = 1;

	if (m_pHandle)
	{
		ret = (int32_t)(SQLITE_OK == sqlite3_close(m_pHandle));
		if (ret)
		{
			m_pHandle = NULL;
		}
	}

	return ret;
}

int32_t CNX_DAudioStatus::IsOpenedDataBase()
{
	return m_pHandle ? 1 : 0;
}

int32_t CNX_DAudioStatus::CreateTable()
{
	int32_t ret = 1;
	char query[1024] = {0,};

	if (!IsOpenedDataBase())
		return 0;

	sprintf(query, "CREATE TABLE IF NOT EXISTS %s (", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE);
	sprintf(query+strlen(query), "%s %s %s,", "_ID", "INT PRIMARY KEY", "NOT NULL");
	sprintf(query+strlen(query), "%s %s %s,", "_VOLUME", "INT", "NOT NULL");
	sprintf(query+strlen(query), "%s %s %s);", "_BT_CONNECTION", "INT", "NOT NULL");

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, NULL, NULL, NULL));
	return ret;
}

int32_t CNX_DAudioStatus::InsertTuple()
{
	int32_t ret = 1;
	char query[1024] = {0,};

	if (!IsOpenedDataBase())
		return 0;

	sprintf(query, "INSERT INTO %s (_ID, _VOLUME, _BT_CONNECTION) VALUES (%d, %d, %d);", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE, 0, 50, 0);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));

	return ret;
}

int32_t CNX_DAudioStatus::RowCount()
{
	char query[1024] = {0,};
	int32_t ret = 0;

	if (!IsOpenedDataBase())
		return 0;

	sprintf(query, "SELECT COUNT(*) FROM %s;", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));
	ret = (ret && g_cbArgs.argv.size() ? atoi(g_cbArgs.argv[0].c_str()) : 0);

	return ret;
}

int32_t CNX_DAudioStatus::IsExistByID(int32_t id)
{
	char query[1024] = {0,};
	int32_t ret = 0;

	if (!IsOpenedDataBase())
		return 0;

	sprintf(query, "SELECT * FROM %s WHERE _ID=%d;", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE, id);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));
	ret = (ret && g_cbArgs.argv.size() ? 1 : 0);

	return ret;
}

int32_t CNX_DAudioStatus::SetBTConnection(int32_t value)
{
	char query[1024] = {0,};
	int32_t ret = 0;

	if (!IsOpenedDataBase())
		return 0;

	sprintf(query, "UPDATE %s set _BT_CONNECTION = %d where _ID = %d;", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE, value, 0);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));

	return ret;
}

int32_t CNX_DAudioStatus::GetBTConnection()
{
	char query[1024] = {0,};
	int32_t ret = 0;

	sprintf(query, "SELECT _BT_CONNECTION FROM %s WHERE _ID=%d;", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE, 0);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));
	ret = (ret && g_cbArgs.argv.size() ? atoi(g_cbArgs.argv[0].c_str()) : -1);

	return ret;
}

int32_t CNX_DAudioStatus::SetVolume(int32_t value)
{
	char query[1024] = {0,};
	int32_t ret = 0;

	if (!IsOpenedDataBase())
		return 0;

	sprintf(query, "UPDATE %s set _VOLUME = %d where _ID = %d;", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE, value, 0);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));

	if (ret)
	{
		int32_t iRet = SetSystemVolume(value);
		if (0 < iRet)
		{
			fprintf(stderr, "SetSystemVolume = %d\n", iRet);
		}
	}

	return ret;
}

int32_t CNX_DAudioStatus::SetSystemVolume(int32_t percentage)
{
	long min, max;
	snd_mixer_t *pHandle;
	snd_mixer_elem_t *pElem;
	snd_mixer_selem_id_t *pSid;
	const char *pCard = m_CardNumber.c_str();
	const char *pSelem_name = m_SoundCard.c_str();
	int32_t iError = 0;
	char card[64] = "default";

	if (!strstr(pCard, card))
	{
		int index = snd_card_get_index(pCard);
		if (index >= 0 && index < 32)
		{
			sprintf(card, "hw:%s", pCard);
		}
	}

	if (percentage < 0)
		percentage = 0;
	if (percentage > 100)
		percentage = 100;

	if (0 > snd_mixer_open(&pHandle, 0))
		return -1;

	if (0 > snd_mixer_attach(pHandle, card))
	{
		iError = -2;
		goto loop_finished;
	}

	if (0 > snd_mixer_selem_register(pHandle, NULL, NULL))
	{
		iError = -2;
		goto loop_finished;
	}

	if (0 > snd_mixer_load(pHandle))
	{
		iError = -2;
		goto loop_finished;
	}

	snd_mixer_selem_id_alloca(&pSid);
	snd_mixer_selem_id_set_index(pSid, 0);
	snd_mixer_selem_id_set_name(pSid, pSelem_name);
	pElem = snd_mixer_find_selem(pHandle, pSid);
	if (!pElem)
	{
		iError = -2;
		goto loop_finished;
	}

	if (m_iMaxVolume > 0)
		max = m_iMaxVolume;
	else if (0 != snd_mixer_selem_get_playback_volume_range(pElem, &min, &max))
	{
		iError = -3;
		goto loop_finished;
	}
	snd_mixer_selem_set_playback_volume_all(pElem, max * percentage / 100);

loop_finished:
	snd_mixer_close(pHandle);

	return iError;
}

int32_t CNX_DAudioStatus::GetVolume()
{
	char query[1024] = {0,};
	int32_t ret = 0;

	sprintf(query, "SELECT _VOLUME FROM %s WHERE _ID=%d;", DEFAULT_DAUDIO_STATUS_DATABASE_TABLE, 0);

	ret = (int32_t)(SQLITE_OK == sqlite3_exec(m_pHandle, query, cbSqlite3Exec, NULL, NULL));
	ret = (ret && g_cbArgs.argv.size() ? atoi(g_cbArgs.argv[0].c_str()) : -1);

	return ret;
}

void *CNX_DAudioStatus::ThreadStub(void *pObj)
{
	if (pObj)
	{
		((CNX_DAudioStatus *)pObj)->ThreadProc();
		pthread_join(((CNX_DAudioStatus *)pObj)->m_hInitThread, NULL);
	}
}

void CNX_DAudioStatus::ThreadProc()
{
	int32_t volume = GetVolume();
	int32_t ret = 0;

	if (volume <= 0)
	{
		return;
	}

	while (true)
	{
		ret = SetSystemVolume(volume);
		if (0 == ret)
		{
			break;
		}
		NXLOGE("[%s] SetSystemVolume() FAILED, ret = %d\n", __PRETTY_FUNCTION__, ret);
		usleep(100000);
	}
}
