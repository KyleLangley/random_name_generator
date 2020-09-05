/* name_generator_buffer.h : September 5th 2020 11:01 am */

#if !defined(NAME_GENERATOR_BUFFER_H)

struct buffer
{
    char* StartPtr;
    char* Ptr;
    int Size;
};

static void IncrementBuffer(buffer& Buffer, const int In)
{
    Buffer.Ptr += In;
}

static void FreeBuffer(buffer& Buffer)
{
    VirtualFree(Buffer.Ptr, 0, MEM_RELEASE);
}

static void AllocBuffer(buffer& Buffer)
{
    if(Buffer.Ptr != nullptr)
    {
        FreeBuffer(Buffer);
    }
    
    if(Buffer.Size > 0)
    {
        Buffer.Ptr = (char*)VirtualAlloc(nullptr, Buffer.Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        Buffer.StartPtr = Buffer.Ptr;
    }
}

static int UsedSizeBuffer(buffer& Buffer)
{
    return Buffer.Ptr - Buffer.StartPtr;
}

#define NAME_GENERATOR_BUFFER_H
#endif
