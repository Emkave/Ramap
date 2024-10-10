#include <ntddk.h>

#define IOCTL_READ_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS)

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING RegistryPath);
void UnloadDriver(PDRIVER_OBJECT pDriverObject);
NTSTATUS DispatchIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING deviceName, symbolicLink;
    PDEVICE_OBJECT pDeviceObject = NULL;
    NTSTATUS status;

    RtlInitUnicodeString(&deviceName, L"\\Device\\PhysMemReader");
    RtlInitUnicodeString(&symbolicLink, L"\\DosDevices\\PhysMemReader");

    status = IoCreateDevice(pDriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(pDeviceObject);
        return status;
    }

    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoControl;
    pDriverObject->DriverUnload = UnloadDriver;

    return STATUS_SUCCESS;
}

void UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
    UNICODE_STRING symbolicLink;
    RtlInitUnicodeString(&symbolicLink, L"\\DosDevices\\PhysMemReader");
    IoDeleteSymbolicLink(&symbolicLink);
    IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS DispatchIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_READ_PHYSICAL_MEMORY:
        {
            PVOID userBuffer = Irp->AssociatedIrp.SystemBuffer;
            PHYSICAL_ADDRESS physAddr;
            physAddr.QuadPart = 0x0;  // Start reading physical memory from 0

            PVOID mappedMemory = MmMapIoSpace(physAddr, 4096, MmNonCached);  // Map 4KB of physical memory

            if (mappedMemory) {
                RtlCopyMemory(userBuffer, mappedMemory, 4096);  // Copy memory to the user buffer
                MmUnmapIoSpace(mappedMemory, 4096);
            } else {
                status = STATUS_UNSUCCESSFUL;
            }
            break;
        }
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
