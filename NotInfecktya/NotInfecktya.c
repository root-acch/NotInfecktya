#include <fltKernel.h>
#include <ntddk.h>
#include <dontuse.h>
#include <wdm.h>
#include <math.h>

#include "ExtMon.h"

ULONG RansomFound = 1645;

ULONG randomToken = 0;
ULONG userToken = 1;

ULONG endOps = 0;
CSHORT previusTime = 0;
ULONG endTime = 0;
ULONG checkTime = 5;

ULONG FilesEntropyChange = 0;

PFLT_PORT port = NULL;
PFLT_PORT ClientPort = NULL;

PFLT_FILTER FilterHandle = NULL;
PDEVICE_OBJECT DeviceObject = NULL;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS InfecktyaPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext);
FLT_POSTOP_CALLBACK_STATUS InfecktyaPostWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext);
FLT_PREOP_CALLBACK_STATUS InfecktyaPreDirectory(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext);
FLT_PREOP_CALLBACK_STATUS InfecktyaPreInformation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext);
FLT_PREOP_CALLBACK_STATUS InfecktyaPreRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext);

const FLT_OPERATION_REGISTRATION Callbacks[] = {
	{IRP_MJ_WRITE,0,InfecktyaPreWrite,InfecktyaPostWrite},
	{IRP_MJ_DIRECTORY_CONTROL, 0, InfecktyaPreDirectory, NULL},
	{IRP_MJ_SET_INFORMATION, 0, InfecktyaPreInformation, NULL},
	{IRP_MJ_CREATE, 0, InfecktyaPreInformation, NULL},
	{IRP_MJ_READ, 0, InfecktyaPreRead, NULL},
	{IRP_MJ_OPERATION_END}
};

const FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	Callbacks,
	MiniUnload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
	DbgPrintEx(0, 0, "[NOTINFECKTYA]: Driver Unload\n");
	FltCloseCommunicationPort(port);
	FltUnregisterFilter(FilterHandle);

	return STATUS_SUCCESS;
}

NTSTATUS IsOnBackup(WCHAR* backupFile) {
	UNICODE_STRING strBackupFile;

	OBJECT_ATTRIBUTES objAttrib;
	HANDLE HFile = NULL;

	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	RtlInitUnicodeString(&strBackupFile, backupFile);
	InitializeObjectAttributes(&objAttrib, &strBackupFile, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwOpenFile(
		&HFile,
		FILE_GENERIC_READ,
		&objAttrib,
		&ioStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
	);

	ZwClose(HFile);

	return status;
}

NTSTATUS CopyFileToDirectory(UNICODE_STRING file, WCHAR* backupFile) {
	UNICODE_STRING strBackupFile;

	OBJECT_ATTRIBUTES sourceObjectAttributes;
	OBJECT_ATTRIBUTES targetObjectAttributes;
	HANDLE sourceFileHandle = NULL;
	HANDLE targetFileHandle = NULL;

	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	InitializeObjectAttributes(&sourceObjectAttributes, &file, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwOpenFile(
		&sourceFileHandle,
		FILE_GENERIC_READ,
		&sourceObjectAttributes,
		&ioStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
	);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to open source file - 0x%x\n", status);
		ZwClose(sourceFileHandle);
		return status;
	}

	RtlInitUnicodeString(&strBackupFile, backupFile);
	InitializeObjectAttributes(&targetObjectAttributes, &strBackupFile, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwCreateFile(
		&targetFileHandle,
		FILE_GENERIC_WRITE,
		&targetObjectAttributes,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to create target file - 0x%x\n", status);
		ZwClose(targetFileHandle);
		ZwClose(sourceFileHandle);
		return status;
	}

	PVOID buffer = ExAllocatePool(NonPagedPool, PAGE_SIZE, "bck");
	LARGE_INTEGER offset = { 0 };
	ULONG len;

	if (buffer == NULL)
	{
		ZwClose(sourceFileHandle);
		ZwClose(targetFileHandle);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	while (TRUE)
	{
		status = ZwReadFile(
			sourceFileHandle,
			NULL,
			NULL,
			NULL,
			&ioStatusBlock,
			buffer,
			PAGE_SIZE,
			&offset,
			NULL
		);

		if (!NT_SUCCESS(status))
		{
			if (STATUS_END_OF_FILE == status)
			{
				status = STATUS_SUCCESS;
			}
			break;
		}

		len = (ULONG)ioStatusBlock.Information;

		status = ZwWriteFile(
			targetFileHandle,
			NULL,
			NULL,
			NULL,
			&ioStatusBlock,
			buffer,
			len,
			&offset,
			NULL
		);

		if (!NT_SUCCESS(status))
		{
			break;
		}

		offset.QuadPart += len;
	}

	ExFreePool(buffer);
	ZwClose(targetFileHandle);
	ZwClose(sourceFileHandle);

	return status;
}

NTSTATUS createDir(WCHAR* dir) {
	NTSTATUS status;

	UNICODE_STRING fileName;
	OBJECT_ATTRIBUTES objAttr;
	HANDLE BackupDir = NULL;

	RtlInitUnicodeString(&fileName, dir);

	InitializeObjectAttributes(&objAttr, &fileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

	IO_STATUS_BLOCK ioStatusBlock;

	status = ZwCreateFile(
		&BackupDir,
		FILE_GENERIC_WRITE,
		&objAttr,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		FILE_CREATE,
		FILE_DIRECTORY_FILE,
		NULL,
		0
	);

	if (status == STATUS_OBJECT_NAME_COLLISION)
	{
		status = ZwOpenFile(
			&BackupDir,
			FILE_GENERIC_READ,
			&objAttr,
			&ioStatusBlock,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			FILE_DIRECTORY_FILE
		);
	}

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Create file failed with status 0x%X\n", status);
		return status;
	}
	else {
		ZwClose(BackupDir);
	}

	return status;
}

ULONG shannonEntropy(UNICODE_STRING file) {
	PVOID buffer = NULL;
	SIZE_T bufferSize = 0;
	ULONG64 histogram[256] = { 0 };
	ULONG totalBytes = 0;
	ULONG bytesRead = 0;

	OBJECT_ATTRIBUTES sourceObjectAttributes;
	HANDLE sourceFileHandle = NULL;

	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	InitializeObjectAttributes(&sourceObjectAttributes, &file, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwOpenFile(
		&sourceFileHandle,
		FILE_GENERIC_READ,
		&sourceObjectAttributes,
		&ioStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
	);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to open source file - 0x%x\n", status);
		ZwClose(sourceFileHandle);
		return status;
	}

	buffer = ExAllocatePool(NonPagedPool, 4096, "bck");
	LARGE_INTEGER offset = { 0 };
	ULONG len;
	ULONG resultIndex = 0;
	UCHAR concatenatedResult[12] = { 0 };

	if (buffer == NULL)
	{
		ZwClose(sourceFileHandle);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	while (TRUE)
	{
		status = ZwReadFile(
			sourceFileHandle,
			NULL,
			NULL,
			NULL,
			&ioStatusBlock,
			buffer,
			PAGE_SIZE,
			&offset,
			NULL
		);

		if (!NT_SUCCESS(status))
		{
			if (STATUS_END_OF_FILE == status)
			{
				status = STATUS_SUCCESS;
			}
			break;
		}

		len = (ULONG)ioStatusBlock.Information;

		totalBytes += len;

		for (ULONG i = 0; i < len; i++) {
			UCHAR byteValue = ((UCHAR*)buffer)[i];

			histogram[byteValue]++;
		}

		offset.QuadPart += len;
	}

	XSTATE_SAVE SaveState;
	NTSTATUS Status;

	Status = KeSaveExtendedProcessorState(XSTATE_MASK_LEGACY, &SaveState);
	if (!NT_SUCCESS(Status)) {
		return status;
	}

	ULONG finalEntropy;
	__try {
		double entropy = 0;
		for (ULONG i = 0; i < 256; i++) {
			if (histogram[i] > 0) {
				double probability = (double)histogram[i] / totalBytes;
				entropy -= probability * (log10(probability) / log10(2));
			}
		}

		finalEntropy = (ULONG)(entropy * 100000);
	}
	__finally {
		KeRestoreExtendedProcessorState(&SaveState);
	}

	ExFreePool(buffer);
	ZwClose(sourceFileHandle);

	return finalEntropy;
}

ULONG magicBytesVerification(const UCHAR* currentRow, ULONG numOfRows, ULONG numOfColumns, const UCHAR* bytes){
	ULONG is_Valid = 0;

	for (int row = 0; row < numOfRows; row++) {
		ULONG match = 1;
		for (int i = 0; i < numOfColumns; i++) {
			DbgPrint("file: 0x%X ", bytes[i]);
			DbgPrint("magic Bytes: 0x%X ", currentRow[i]);
			if (bytes[i] != currentRow[i]) {
				match = 0;
				break;
			}
		}

		if (match) {
			is_Valid = 1;
			break;
		}
	}

	DbgPrint("\n");

	return is_Valid;
}

ULONG magicBytesCheck(UNICODE_STRING file) {
	PVOID buffer = NULL;
	SIZE_T bufferSize = 0;

	ULONG bytesRead = 0;

	OBJECT_ATTRIBUTES sourceObjectAttributes;
	HANDLE sourceFileHandle = NULL;

	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	InitializeObjectAttributes(&sourceObjectAttributes, &file, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwOpenFile(
		&sourceFileHandle,
		FILE_GENERIC_READ,
		&sourceObjectAttributes,
		&ioStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
	);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to open source file on magic bytes - 0x%x\n", status);
		ZwClose(sourceFileHandle);
		return status;
	}

	buffer = ExAllocatePool(NonPagedPool, 4096, "bck");
	LARGE_INTEGER offset = { 0 };
	ULONG len;
	ULONG resultIndex = 0;
	UCHAR concatenatedResult[12] = { 0 };

	if (buffer == NULL)
	{
		ZwClose(sourceFileHandle);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	while (TRUE)
	{
		status = ZwReadFile(
			sourceFileHandle,
			NULL,
			NULL,
			NULL,
			&ioStatusBlock,
			buffer,
			PAGE_SIZE,
			&offset,
			NULL
		);

		if (!NT_SUCCESS(status))
		{
			if (STATUS_END_OF_FILE == status)
			{
				status = STATUS_SUCCESS;
			}
			break;
		}

		len = (ULONG)ioStatusBlock.Information;

		for (ULONG i = 0; i < len; i++) {
			if (resultIndex <= 13) {
				UCHAR byteValue = ((UCHAR*)buffer)[i];

				UCHAR highNibble = (byteValue >> 4) & 0x0F;
				UCHAR lowNibble = byteValue & 0x0F;
				UCHAR concatenatedByte = (highNibble << 4) | lowNibble;
				concatenatedResult[resultIndex] = concatenatedByte;
				resultIndex++;
			}
		}

		offset.QuadPart += len;
	}

	ExFreePool(buffer);
	ZwClose(sourceFileHandle);

	ULONG is_Valid = magicBytesVerification(magicBytes1, 5, 4, concatenatedResult);
	if (is_Valid == 1) return 1;
	
	is_Valid = magicBytesVerification(magicBytes2, 4, 8, concatenatedResult);
	if (is_Valid == 1) return 1;

	is_Valid = magicBytesVerification(magicBytes3, 1, 2, concatenatedResult);
	if (is_Valid == 1) return 1;

	is_Valid = magicBytesVerification(magicBytes5, 1, 10, concatenatedResult);
	if (is_Valid == 1) return 1;

	is_Valid = magicBytesVerification(magicBytes6, 1, 7, concatenatedResult);
	if (is_Valid == 1) return 1;

	is_Valid = magicBytesVerification(magicBytes7, 1, 12, concatenatedResult);
	if (is_Valid == 1) return 1;

	return 0;
}

NTSTATUS ListFilesInDirectory(WCHAR* directoryPath, WCHAR* recoverDir)
{
	PVOID buffer = NULL;
	UNICODE_STRING unicodeDirectoryPath;
	OBJECT_ATTRIBUTES objectAttributes;
	HANDLE directoryHandle;
	NTSTATUS status;
	IO_STATUS_BLOCK ioStatusBlock;

	OBJECT_ATTRIBUTES BackupFileobjAttrib;

	RtlInitUnicodeString(&unicodeDirectoryPath, directoryPath);

	InitializeObjectAttributes(&objectAttributes, &unicodeDirectoryPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwOpenFile(
		&directoryHandle,
		FILE_LIST_DIRECTORY | SYNCHRONIZE,
		&objectAttributes,
		&ioStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
	);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to open dir to query - 0x%x\n", status);
		return status;
	}

	ULONG bufferSize = PAGE_SIZE;
	buffer = ExAllocatePool(NonPagedPool, bufferSize, 'bck');

	if (buffer == NULL)
	{
		ZwClose(directoryHandle);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	BOOLEAN restartScan = TRUE;
	PFILE_DIRECTORY_INFORMATION fileInfo = (PFILE_DIRECTORY_INFORMATION)buffer;

	do {
		status = ZwQueryDirectoryFile(directoryHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, bufferSize, FileDirectoryInformation, restartScan, NULL, FALSE);

		if (NT_SUCCESS(status)) {
			fileInfo = (PFILE_DIRECTORY_INFORMATION)buffer;

			while (TRUE) {
				UNICODE_STRING fileName;
				RtlInitUnicodeString(&fileName, fileInfo->FileName);

				if (fileName.Length > 0) {
					if (fileInfo->FileNameLength > 0 && (!(fileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))) {
						PWSTR fileNameBuffer = ExAllocatePool(NonPagedPool, fileInfo->FileNameLength * sizeof(WCHAR), 'fnm');

						if (fileNameBuffer != NULL) {
							RtlCopyMemory(fileNameBuffer, fileInfo->FileName, fileInfo->FileNameLength);
							fileNameBuffer[fileInfo->FileNameLength / sizeof(WCHAR)] = L'\0';

							WCHAR backupFile[512] = { 0 };
							wcscpy(backupFile, directoryPath);
							wcscat(backupFile, fileNameBuffer);

							UNICODE_STRING fileBackupName;
							RtlInitUnicodeString(&fileBackupName, backupFile);

							WCHAR recoverFile[512] = { 0 };
							wcscpy(recoverFile, recoverDir);
							wcscat(recoverFile, fileNameBuffer);

							CopyFileToDirectory(fileBackupName, recoverFile);

							InitializeObjectAttributes(&BackupFileobjAttrib, &fileBackupName, OBJ_CASE_INSENSITIVE, NULL, NULL);

							ZwDeleteFile(&BackupFileobjAttrib);

							ExFreePool(fileNameBuffer);
						}
					}
				}

				if (fileInfo->NextEntryOffset == 0) {
					break;
				}

				fileInfo = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)fileInfo + fileInfo->NextEntryOffset);
			}

			restartScan = FALSE;
		}
	} while (status == STATUS_SUCCESS);

	ExFreePool(buffer);
	ZwClose(directoryHandle);

	return STATUS_SUCCESS;
}

NTSTATUS recoverBackup() {
	NTSTATUS status;

	status = createDir(L"\\??\\C:\\Backup_Recovery");

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to create recover directory\n");
		return status;
	}

	ListFilesInDirectory(L"\\??\\C:\\BackFromDead\\", L"\\??\\C:\\Backup_Recovery\\");

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to recover from Backup\n");
		return status;
	}

	return status;
}

NTSTATUS killProcess(ULONG pid) {
	NTSTATUS status;
	HANDLE hProcess;

	HANDLE process = ULongToHandle(pid);
	PEPROCESS thehandle;
	status = PsLookupProcessByProcessId(process, &thehandle);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	status = ObOpenObjectByPointer(thehandle, 0, NULL, 0, NULL, KernelMode, &hProcess);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	status = ZwTerminateProcess(hProcess, STATUS_ACCESS_DENIED);
	if (!NT_SUCCESS(status)) {
		ZwClose(hProcess);
		return status;
	}
	ZwClose(hProcess);

	return status;
}

FLT_PREOP_CALLBACK_STATUS InfecktyaPreRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext) {
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(status)) {
			//HoneyPot
			if (wcsstr(FileNameInfo->Name.Buffer, L".__Infecktya") != NULL || wcsstr(FileNameInfo->Name.Buffer, L"aaInfecktya") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L"zzInfecktya") != NULL || wcsstr(FileNameInfo->Name.Buffer, L".__Infecktya.txt") != NULL 
				|| wcsstr(FileNameInfo->Name.Buffer, L"aaInfecktya.txt") != NULL || wcsstr(FileNameInfo->Name.Buffer, L"zzInfecktya.txt") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L".__Infecktya.docx") != NULL || wcsstr(FileNameInfo->Name.Buffer, L"aaInfecktya.docx") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L"zzInfecktya.docx") != NULL){

				ULONG pid = FltGetRequestorProcessId(Data);
				status = killProcess(pid);

				FltReleaseFileNameInformation(FileNameInfo);
				return FLT_PREOP_COMPLETE;
			}
		}

		FltReleaseFileNameInformation(FileNameInfo);
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS InfecktyaPreInformation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext) {
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		//blackList
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Extension.Length > 0) {
				const WCHAR* currentExtension = FileNameInfo->Extension.Buffer;

				for (ULONG i = 0; i < (sizeof(monitoredExtensions) / sizeof(monitoredExtensions[0])); ++i) {
					if (_wcsicmp(currentExtension, monitoredExtensions[i]) == 0) {
						status = ZwTerminateProcess(ZwCurrentProcess(), 0);

						FltReleaseFileNameInformation(FileNameInfo);
						return FLT_PREOP_COMPLETE;
					}
				}

				FltReleaseFileNameInformation(FileNameInfo);
			}
		}
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS InfecktyaPreDirectory(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext) {
	PFLT_FILE_NAME_INFORMATION DirNameInfo;
	NTSTATUS status;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &DirNameInfo);

	if (NT_SUCCESS(status)) {
		if (wcsstr(DirNameInfo->Name.Buffer, L"BackFromDead") != NULL) {
			if (Data->RequestorMode != KernelMode) {
				FltReleaseFileNameInformation(DirNameInfo);
				return FLT_PREOP_COMPLETE;
			}
		}
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS InfecktyaPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext) {
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	UNICODE_STRING targetDirectoryPath;
	NTSTATUS status;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Extension.Length > 0) {
				//BlackList
				const WCHAR* currentExtension = FileNameInfo->Extension.Buffer;

				for (ULONG i = 0; i < (sizeof(monitoredExtensions) / sizeof(monitoredExtensions[0])); ++i) {
					if (_wcsicmp(currentExtension, monitoredExtensions[i]) == 0) {

						ULONG pid = FltGetRequestorProcessId(Data);
						status = killProcess(pid);

						FltReleaseFileNameInformation(FileNameInfo);
						return FLT_PREOP_COMPLETE;
					}
				}
			}

			//HoneyPot
			if (wcsstr(FileNameInfo->Name.Buffer, L".__Infecktya") != NULL || wcsstr(FileNameInfo->Name.Buffer, L"aaInfecktya") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L"zzInfecktya") != NULL || wcsstr(FileNameInfo->Name.Buffer, L".__Infecktya.txt") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L"aaInfecktya.txt") != NULL || wcsstr(FileNameInfo->Name.Buffer, L"zzInfecktya.txt") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L".__Infecktya.docx") != NULL || wcsstr(FileNameInfo->Name.Buffer, L"aaInfecktya.docx") != NULL
				|| wcsstr(FileNameInfo->Name.Buffer, L"zzInfecktya.docx") != NULL) {

				ULONG pid = FltGetRequestorProcessId(Data);
				status = killProcess(pid);

				FltReleaseFileNameInformation(FileNameInfo);
				return FLT_PREOP_COMPLETE;
			}

			//Entropy
			if (FileNameInfo->Extension.Length > 0) {
				const WCHAR* finalArchive = FileNameInfo->FinalComponent.Buffer;

				for (ULONG i = 0; i < (sizeof(EntropyExtensions) / sizeof(EntropyExtensions[0])); ++i) {
					if (wcsstr(FileNameInfo->Name.Buffer, L"\\??\\C:\\BackFromDead\\") == NULL && wcsstr(finalArchive, EntropyExtensions[i]) != NULL) {
						WCHAR backupFile[512] = { 0 };
						wcscpy(backupFile, L"\\??\\C:\\BackFromDead\\");
						wcscat(backupFile, FileNameInfo->FinalComponent.Buffer);

						status = IsOnBackup(backupFile);

						if (!NT_SUCCESS(status)) {
							DbgPrint("[NOTINFECKTYA]: file isn't on backup\n");
							status = CopyFileToDirectory(FileNameInfo->Name, backupFile);

							if (NT_SUCCESS(status)) {
								DbgPrint("[NOTINFECKTYA]: backup complete!\n");
							}
						}
					}
				}
			}
		}

		if (FileNameInfo != NULL) {
			FltReleaseFileNameInformation(FileNameInfo);
		}
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS InfecktyaPostWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS fltObjects, PVOID* CompletionContext)
{
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	UNICODE_STRING targetDirectoryPath;
	NTSTATUS status;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Extension.Length > 0) {
				const WCHAR* finalArchive = FileNameInfo->FinalComponent.Buffer;

				for (ULONG i = 0; i < (sizeof(EntropyExtensions) / sizeof(EntropyExtensions[0])); ++i) {
					if (wcsstr(FileNameInfo->Name.Buffer, L"\\??\\C:\\BackFromDead\\") == NULL && wcsstr(finalArchive, EntropyExtensions[i]) != NULL) {
						WCHAR backupFile[512] = { 0 };
						wcscpy(backupFile, L"\\??\\C:\\BackFromDead\\");
						wcscat(backupFile, FileNameInfo->FinalComponent.Buffer);

						UNICODE_STRING unistrBackupFile;
						RtlInitUnicodeString(&unistrBackupFile, backupFile);

						ULONG newFile = shannonEntropy(FileNameInfo->Name);
						ULONG backup = shannonEntropy(unistrBackupFile);

						if (backup == 0 || backup == newFile) {
							break;
						}

						DbgPrint("new: %lu\n backup: %lu\n", newFile, backup);

						if (newFile != backup) {
							if (magicBytesCheck(FileNameInfo->Name) == 0) {
								ULONG pid = FltGetRequestorProcessId(Data);
								status = killProcess(pid);

								if (FileNameInfo != NULL) {
									FltReleaseFileNameInformation(FileNameInfo);
								}

								return FLT_POSTOP_FINISHED_PROCESSING;
							}
							else {
								LARGE_INTEGER currentTime;
								TIME_FIELDS timeFields;

								KeQuerySystemTime(&currentTime);
								RtlTimeToTimeFields(&currentTime, &timeFields);

								CSHORT seconds_passed = (timeFields.Second + 60 - previusTime) % 60;

								if (seconds_passed <= checkTime) {
									if (newFile >= backup + 1000) {
										FilesEntropyChange++;
									}

									if (FilesEntropyChange >= 2 && randomToken != userToken) {
										ULONG pid = FltGetRequestorProcessId(Data);
										status = killProcess(pid);
									}
								}
								else {
									previusTime = timeFields.Second;
								}
							}
						}
					}
				}
			}
		}

		if (FileNameInfo != NULL) {
			FltReleaseFileNameInformation(FileNameInfo);
		}
	}

	return FLT_POSTOP_FINISHED_PROCESSING;
}

NTSTATUS InfecktyaConnect(PFLT_PORT clientPort, PVOID serverPortCookie, PVOID Context, ULONG size, PVOID ConnectionCookie) {
	ClientPort = clientPort;
	DbgPrintEx(0, 0, "[NOTINFECKTYA]: Connect\n");
	return STATUS_SUCCESS;
}

void InfecktyaDisconnect(PVOID connectionCookie) {
	DbgPrintEx(0, 0, "[NOTINFECKTYA]: Disconnect\n");
	FltCloseClientPort(FilterHandle, &ClientPort);
}

NTSTATUS InfecktyaSendRec(PVOID portCookie, PVOID inputBuffer, ULONG InputBufferLength, PVOID outputBuffer, ULONG OutputBufferLength, PULONG RetLength)
{
	int userTokenReceive = *(int*)inputBuffer;
	userToken = userTokenReceive;
	DbgPrintEx(0, 0, "[NOTINFECKTYA]: Get Initial Code -> %lu\n", userToken);

	DbgPrintEx(0, 0, "[NOTINFECKTYA]: Send Initial Code -> %lu\n", randomToken);
	RtlCopyMemory(outputBuffer, &randomToken, sizeof(ULONG));
	*RetLength = sizeof(ULONG);

	return STATUS_SUCCESS;
}

ULONG GenerateRandomNumber()
{
	ULONG seed = KeQueryPerformanceCounter(NULL).LowPart;
	RtlRandomEx(&seed);

	return seed;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status;
	PSECURITY_DESCRIPTOR SecDesc;
	OBJECT_ATTRIBUTES oa = { 0 };
	UNICODE_STRING comName = RTL_CONSTANT_STRING(L"\\NotInfecktya");

	status = createDir(L"\\??\\C:\\BackFromDead");

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to create Backup\n");
		return status;
	}

	LARGE_INTEGER currentTime;
	TIME_FIELDS timeFields;

	KeQuerySystemTime(&currentTime);
	RtlTimeToTimeFields(&currentTime, &timeFields);
	previusTime = timeFields.Second;

	randomToken = GenerateRandomNumber();

	status = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterHandle);

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(0, 0, "[NOTINFECKTYA]: Failed to create Notify\n");
		return status;
	}

	status = FltBuildDefaultSecurityDescriptor(&SecDesc, FLT_PORT_ALL_ACCESS);

	if (NT_SUCCESS(status)) {
		InitializeObjectAttributes(&oa, &comName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, SecDesc);
		FltCreateCommunicationPort(FilterHandle, &port, &oa, NULL, InfecktyaConnect,
			InfecktyaDisconnect, InfecktyaSendRec, 1);

		FltFreeSecurityDescriptor(SecDesc);

		if (NT_SUCCESS(status)) {

			status = FltStartFiltering(FilterHandle);

			if (NT_SUCCESS(status)) {
				DbgPrintEx(0, 0, "[NOTINFECKTYA]: Driver Loaded\n");
				return status;
			}

			FltCloseCommunicationPort(port);
		}
	}

	FltUnregisterFilter(FilterHandle);

	return status;
}