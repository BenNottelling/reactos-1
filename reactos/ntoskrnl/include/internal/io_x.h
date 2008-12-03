/*
* PROJECT:         ReactOS Kernel
* LICENSE:         GPL - See COPYING in the top level directory
* FILE:            ntoskrnl/include/io_x.h
* PURPOSE:         Internal Inlined Functions for the I/O Manager
* PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
*/

static
__inline
VOID
IopLockFileObject(IN PFILE_OBJECT FileObject)
{
	/* Lock the FO and check for contention */
    InterlockedIncrement((PLONG)&FileObject->Waiters);
    while (InterlockedCompareExchange((PLONG)&FileObject->Busy, TRUE, FALSE) != FALSE)
    {
        /* FIXME - pause for a little while? */
    }
    InterlockedDecrement((PLONG)&FileObject->Waiters);
}

static
__inline
VOID
IopUnlockFileObject(IN PFILE_OBJECT FileObject)
{
    /* Unlock the FO and wake any waiters up */
    InterlockedExchange((PLONG)&FileObject->Busy, FALSE);
    if (FileObject->Waiters) KeSetEvent(&FileObject->Lock, 0, FALSE);
}

FORCEINLINE
VOID
IopQueueIrpToThread(IN PIRP Irp)
{
    KIRQL OldIrql;

    /* Raise to APC Level */
    KeRaiseIrql(APC_LEVEL, &OldIrql);

    /* Insert it into the list */
    InsertHeadList(&Irp->Tail.Overlay.Thread->IrpList, &Irp->ThreadListEntry);

    /* Lower irql */
    KeLowerIrql(OldIrql);
}

FORCEINLINE
VOID
IopUnQueueIrpFromThread(IN PIRP Irp)
{
    /* Remove it from the list and reset it */
    RemoveEntryList(&Irp->ThreadListEntry);
    InitializeListHead(&Irp->ThreadListEntry);
}

static
__inline
VOID
IopUpdateOperationCount(IN IOP_TRANSFER_TYPE Type)
{
    PLARGE_INTEGER CountToChange;

    /* Make sure I/O operations are being counted */
    if (IoCountOperations)
    {
        if (Type == IopReadTransfer)
        {
            /* Increase read count */
            IoReadOperationCount++;
            CountToChange = &PsGetCurrentProcess()->ReadOperationCount;
        }
        else if (Type == IopWriteTransfer)
        {
            /* Increase write count */
            IoWriteOperationCount++;
            CountToChange = &PsGetCurrentProcess()->ReadOperationCount;
        }
        else
        {
            /* Increase other count */
            IoOtherOperationCount++;
            CountToChange = &PsGetCurrentProcess()->ReadOperationCount;
        }

        /* Increase the process-wide count */
        ExInterlockedAddLargeStatistic(CountToChange, 1);
    }
}

static
__inline
BOOLEAN
IopValidateOpenPacket(IN POPEN_PACKET OpenPacket)
{
    /* Validate the packet */
    if (!(OpenPacket) ||
        (OpenPacket->Type != IO_TYPE_OPEN_PACKET) ||
        (OpenPacket->Size != sizeof(OPEN_PACKET)))
    {
        /* Fail */
        return FALSE;
    }

    /* Good packet */
    return TRUE;
}
