#include "UpdateCallLogThread.h"

UpdateCallLogThread::UpdateCallLogThread()
{

}

void UpdateCallLogThread::SetFile(QString path)
{
	m_File = path;
}

void UpdateCallLogThread::Start()
{
	if (isRunning())
	{
		m_bRunning = false;
		quit();
		wait();
	}

	start();
}

void UpdateCallLogThread::run()
{
	m_bRunning = true;

	VCardReader reader;
	if (reader.read(m_File.toStdString()))
	{
		std::vector<VCardReader::VCardProperty> properties = reader.properties();
		QString direction;
		QString owner;
		QString type;

		vector<CallLogInfo> sInfoList;
		CallLogInfo sInfo;

		for (size_t i = 0; i < properties.size() && m_bRunning; ++i)
		{
			if (properties[i].TEL.size() == 0)
				continue;

			switch (properties[i].X_IRMC_CALL_DATETIME.type) {
			case VCardReader::CallType_Dialed:
				direction = "S";
				break;

			case VCardReader::CallType_Receivced:
				direction = "R";
				break;

			case VCardReader::CallType_Missed:
				direction = "M";
				break;

			default: // assume call log empty!
				continue;
			}

			if (properties[i].FN.empty()) {
				owner = QString::fromStdString(properties[i].TEL[0].number);
			} else {
				owner = QString::fromStdString(properties[i].FN);
			}

			switch (properties[i].TEL[0].type) {
			case VCardReader::TelephoneType_Voice:
				type = tr("Voice");
				break;

			case VCardReader::TelephoneType_Home:
				type = tr("Home");
				break;

			case VCardReader::TelephoneType_Cell:
				type = tr("Cell");
				break;

			case VCardReader::TelephoneType_Work:
				type = tr("Work");
				break;

			case VCardReader::TelephoneType_Tel:
				type = tr("Tel");
				break;

			case VCardReader::TelephoneType_Etc:
				type = tr("Etc");
				break;

			default:
				type = tr("Unknown");
				break;
			}

			sInfo.direction = direction;
			sInfo.owner = owner;
			sInfo.type = type;
			sInfoList.push_back(sInfo);

			if (sInfoList.size() >= 5)
			{
				emit signalAdd(sInfoList);
				sInfoList.clear();
				msleep(100);
			}
		}

		if (sInfoList.size() > 0)
		{
			emit signalAdd(sInfoList);
		}
	}

	emit signalCompleted();

	m_bRunning = false;
}
