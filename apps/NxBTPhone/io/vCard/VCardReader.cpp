#include "VCardReader.h"

VCardReader::VCardReader()
{

}

VCardReader::VCardReader(std::string path)
{
    read(path);
}

bool VCardReader::read(std::string path)
{
    std::ifstream file;
    std::string line;
    std::vector<std::string> lines;
    bool enter_group = false;
    char c;
    // reset data
    m_Properties.clear();

    file.open(path.c_str(), ios_base::in);

    if (!file.is_open()) {
        return false;
    }

    while (file.get(c)) {
        if (!(c == '\r' || c == '\n'))
            line += c;

        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        }

        if (!enter_group && line.find("BEGIN:VCARD") == 0) {
            enter_group = true;
            lines.clear();
        } else if (enter_group && line.find("END:VCARD") == 0) {
            enter_group = false;
            m_Properties.push_back(createProperty(lines));
        }
    }

    file.close();
    return true;
}

VCardReader::VCardProperty VCardReader::createProperty(std::vector<std::string>& lines)
{
    VCardProperty p;
    int start = -1;
    int pos = -1;
    std::string temp;

    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].find("VERSION:") == 0) {
            pos = strlen("VERSION:");
            p.VERSION = lines[i].substr(pos, lines[i].length()-pos);
        } else if (lines[i].find("FN") == 0) {
            // find ':'
            pos = lines[i].find(":", 2) + 1;
            p.FN = lines[i].substr(pos, lines[i].length()-pos);
        } else if (lines[i].find("N") == 0) {
            start = lines[i].find(":", 1) + 1;
            pos = lines[i].find(";", start);

            if (pos < 0) {
                p.N.first = lines[i].substr(start, lines[i].length()-pos);
                p.N.last.clear();
            } else {
                p.N.first = lines[i].substr(start, lines[i].length()-pos);
                ++pos;
                p.N.last = lines[i].substr(pos, lines[i].length()-pos);
            }

        } else if (lines[i].find("TEL") == 0) {
            telephone_t T;
            start = lines[i].find(";TYPE", strlen("TEL"));
            if (start >= 0) {
                start = lines[i].find("=");
                if (start < 0)
                    continue;
                ++start;
                pos = lines[i].find(":", 8);
                if (pos > 0) {
                    temp = lines[i].substr(start, pos-start);
                    if (temp == "HOME") {
                        T.type = TelephoneType_Home;
                    } else if (temp == "CELL") {
                        T.type = TelephoneType_Cell;
                    } else if (temp == "WORK") {
                        T.type = TelephoneType_Work;
                    } else if (temp == "VOICE") {
                        T.type = TelephoneType_Voice;
                    } else {
                        T.type = TelephoneType_Etc;
                    }
                }
            } else {
                T.type = TelephoneType_Tel;
            }

            pos = lines[i].find(":");
            if (pos < 0)
                continue;

            // parse telephone number
            ++pos;
            T.number = lines[i].substr(pos, lines[i].length()-pos);

            p.TEL.push_back(T);
        } else if (lines[i].find("ORG:") == 0) {
            pos = strlen("ORG:");
            p.ORG = lines[i].substr(pos, lines[i].length()-pos);
        } else if (lines[i].find("UID:") == 0) {
            pos = strlen("UID:");
            p.UID = lines[i].substr(pos, lines[i].length()-pos);
        } else if (lines[i].find("X-IRMC-CALL-DATETIME") == 0) {
            int pos2 = 0;
            start = strlen("X-IRMC-CALL-DATETIME");

            // find type
            start = lines[i].find(";", start);
            pos = lines[i].find(":", start);
            if (start < 0 || pos < 0)
                continue;

            ++start;
            temp = lines[i].substr(start, pos-start);
            pos2 = temp.find("=") + 1;
            if (pos2 > 0) {
                temp = temp.substr(pos2, pos - pos2);
            }

            if (temp == "DIALED") {
                p.X_IRMC_CALL_DATETIME.type = CallType_Dialed;
            } else if (temp == "RECEIVED") {
                p.X_IRMC_CALL_DATETIME.type = CallType_Receivced;
            } else if (temp == "MISSED") {
                p.X_IRMC_CALL_DATETIME.type = CallType_Missed;
            }

            // find date/time
            ++pos;
            p.X_IRMC_CALL_DATETIME.datetime = lines[i].substr(pos, lines[i].length()-pos);
        } else if (lines[i].find("EMAIL") == 0) {
            email_t T;
            start = lines[i].find(";TYPE", strlen("EMAIL"));
            if (start >= 0) {
                start = lines[i].find("=", 10);
                if (start < 0)
                    continue;
                ++start;
                pos = lines[i].find(":", start);
                if (pos > 0) {
                    temp = lines[i].substr(start, pos-start);
                    if (temp == "INTERNET") {
                        T.type = E_MailType_internet;
                    } else {
                        cerr << "it's not default type:" << temp << endl;
                    }
                }
            }

            pos = lines[i].find(":");
            if (pos < 0)
                continue;

            // parse e-mail address
            ++pos;
            T.address = lines[i].substr(pos, lines[i].length()-pos);

            p.EMAIL.push_back(T);
        }

    }

    return p;
}

std::vector<VCardReader::VCardProperty> VCardReader::properties()
{
    return m_Properties;
}
