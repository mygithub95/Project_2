#include "WinUtilities.h"

#include <QString>
#include <QFileInfo>
#include <winternl.h>

typedef struct _PROCESS_EXTENDED_BASIC_INFORMATION {
    SIZE_T Size;    // Ignored as input, written with structure size on output
    PROCESS_BASIC_INFORMATION BasicInfo;
    union {
        ULONG Flags;
        struct {
            ULONG IsProtectedProcess : 1;
            ULONG IsWow64Process : 1;
            ULONG IsProcessDeleting : 1;
            ULONG IsCrossSessionCreate : 1;
            ULONG IsFrozen : 1;
            ULONG IsBackground : 1;
            ULONG IsStronglyNamed : 1;
            ULONG IsSecureProcess : 1;
            ULONG IsSubsystemProcess : 1;
            ULONG SpareBits : 23;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
} PROCESS_EXTENDED_BASIC_INFORMATION, *PPROCESS_EXTENDED_BASIC_INFORMATION;


QString getProcessExeNameByID(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);

    if(processHandle){
        DWORD pathHolderSize = MAX_PATH;
        wchar_t *pathHolder = new wchar_t[pathHolderSize + 1]();
        QueryFullProcessImageName(processHandle, 0, pathHolder, &pathHolderSize);
        QString exeFilePath(QString::fromWCharArray(pathHolder));
        delete [] pathHolder;
        QString processExe = QFileInfo(exeFilePath).absoluteFilePath();//.split('/').last();

        PROCESS_EXTENDED_BASIC_INFORMATION pebi;
        if (0 <= NtQueryInformationProcess(processHandle, ProcessBasicInformation, &pebi, sizeof(pebi), 0) &&
                pebi.Size >= sizeof(pebi))
        {
            if (pebi.IsFrozen)
            {
                //qDebug() << processExe << "  is FROZEN";
                return QString();
            }
        }
        return processExe;
    }
    return QString();
}
