/*
 * Report how much space is available on the heap.
 * Works with Teensy 3.0/3.1/3.2 and Teensy LC
 *
 * copied from:
 * https://forum.pjrc.com/threads/23256-Get-Free-Memory-for-Teensy-3-0?s=0cd48c6095ec569e3c82f77771aa4ee8&p=34242&viewfull=1#post34242
 */
uint32_t FreeRam(){
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is the free, available ram.
    return stackTop - heapTop;
}
