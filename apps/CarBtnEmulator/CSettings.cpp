#include <stdio.h>
#include "CSettings.h"


class CSettings
{
public:
	CSettings();
	int32_t GetSettings(BTN_SETTINGS **ppSettings);
	int32_t SetSettings(BTN_SETTINGS *pSettings);

	static CSettings *GetInstance()
	{
		if( NULL == m_stSettings )
		{
			m_stSettings = new CSettings();
		}
		return m_stSettings;
	}

private:
	static CSettings *m_stSettings;
	BTN_SETTINGS	m_Setting;
};

CSettings *CSettings::m_stSettings = NULL;

CSettings::CSettings()
{
}

int32_t CSettings::GetSettings(BTN_SETTINGS **ppSettings)
{
	*ppSettings = &m_Setting;
	return 0;
}

int32_t CSettings::SetSettings(BTN_SETTINGS *pSettings)
{
	m_Setting = *pSettings;
	return 0;
}


int32_t GetSettings(BTN_SETTINGS **ppSettings)
{
	CSettings *pObj = CSettings::GetInstance();
	return pObj->GetSettings(ppSettings);
}

int32_t SetSettings(BTN_SETTINGS *pSettings)
{
	CSettings *pObj = CSettings::GetInstance();
	return pObj->SetSettings(pSettings);
}
