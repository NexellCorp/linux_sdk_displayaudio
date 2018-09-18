#include "CNX_DAudioStatus.h"

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
		SetSystemVolume(value);

	return ret;
}

int32_t CNX_DAudioStatus::SetSystemVolume(int32_t percentage)
{
	long min, max;
	snd_mixer_t *pHandle;
	snd_mixer_elem_t *pElem;
	snd_mixer_selem_id_t *pSid;
	const char *pCard = "default";
	const char *pSelem_name = "DAC1";

	if (percentage < 0)
		percentage = 0;
	if (percentage > 100)
		percentage = 100;

	if (0 != snd_mixer_open(&pHandle, 0))
		return -1;

	if (0 != snd_mixer_attach(pHandle, pCard))
		return -1;

	if (0 != snd_mixer_selem_register(pHandle, NULL, NULL))
		return -1;

	if (0 != snd_mixer_load(pHandle))
		return -1;

	snd_mixer_selem_id_alloca(&pSid);
	snd_mixer_selem_id_set_index(pSid, 0);
	snd_mixer_selem_id_set_name(pSid, pSelem_name);
	pElem = snd_mixer_find_selem(pHandle, pSid);

	snd_mixer_selem_get_playback_volume_range(pElem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(pElem, max * percentage / 100);

	snd_mixer_close(pHandle);

	return 0;
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
