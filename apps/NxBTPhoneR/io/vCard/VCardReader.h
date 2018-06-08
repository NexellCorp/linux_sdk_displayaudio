#ifndef VCARDREADER_H
#define VCARDREADER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

#include <string.h>

class VCardReader
{
public:
    struct name_t {
        std::string first;
        std::string last;
    };

    enum TelephoneType {
        TelephoneType_Voice, // default
        TelephoneType_Home,
        TelephoneType_Cell,
        TelephoneType_Work,
        TelephoneType_Tel,
        TelephoneType_Etc,
        TelephoneType_Count
    };

    struct telephone_t {
        TelephoneType type;
        std::string number;
    };

    enum E_MailType {
        E_MailType_internet, // default
        E_MailType_aol,
        E_MailType_applelink,
        E_MailType_attmail,
        E_MailType_cis,
        E_MailType_eworld,
        E_MailType_ibmmail,
        E_MailType_mcimail,
        E_MailType_powershare,
        E_MailType_prodigy,
        E_MailType_tlx,
        E_MailType_x400,
        E_MailType_Count
    };

    struct email_t {
        E_MailType type;
        std::string address;
    };

    enum CallType {
        CallType_None,
        CallType_Dialed,
        CallType_Receivced,
        CallType_Missed
    };

    struct x_irmc_call_datetime_t {
        CallType type;
        std::string datetime;
    };

    struct VCardProperty {
        std::string VERSION;
        std::string FN;
        name_t N;
        std::vector<telephone_t> TEL;
        std::vector<email_t> EMAIL;
        std::string ORG;
        std::string UID;
        x_irmc_call_datetime_t X_IRMC_CALL_DATETIME;
    };

public:
    VCardReader();

    VCardReader(std::string path);

    bool read(std::string path);

    std::vector<VCardProperty> properties();

private:
    VCardProperty createProperty(std::vector<std::string>& blob);

private:
    std::vector<VCardProperty> m_Properties;

};

#endif // VCARDREADER_H
