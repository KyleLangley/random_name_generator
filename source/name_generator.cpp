
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#define STB_DEFINE
#include "stb/stb.h"

#include "name_generator_buffer.h"
#include "name_generator_dictionary.h"

// define
static const int MAX_PATH_LENGTH = 255;
static const char* CONTENT_PATH = "\\content\\";

enum commands
{
    C_Generate,
    C_GenerateFrom,
    C_Copy,
    C_Close,
    C_Help,
    
    C_MAX,
};

struct word_file
{
    char** Buffer;
    int BufferCount;
};

struct list
{
    word_file* Files;
    int FileCount;
};

static char** StoragePtr;
static char** StorageStartPtr;
static const int StorageBufferSize = 1024 * 1024 * 1024;

struct generate
{
    const char* Commands[C_MAX] = {"generate", "from", "copy", "close", "help"};
    char** Directories = nullptr;
    LARGE_INTEGER Frequency;
    list Lists;
    buffer NamesBuffer = {nullptr, nullptr, 1024 * 1024 * 1024};
};

// -- define

static generate Generate;

// functions

static LARGE_INTEGER GetQueryPerformanceCounter()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

static double GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    return ((double)(End.QuadPart - Start.QuadPart) / (double)Generate.Frequency.QuadPart); 
}

static char* GetContentDirectory()
{
    static char PathBuffer[MAX_PATH_LENGTH];
    if(DWORD PathLength = GetCurrentDirectory(MAX_PATH_LENGTH, &PathBuffer[0]))
    {
        if(char* Ptr = strstr(&PathBuffer[0], "\\build"))
        {
            memcpy(Ptr, CONTENT_PATH, strlen(CONTENT_PATH));
            return &PathBuffer[0];
        }
    }
    
    return nullptr;
}

static unsigned long GetRand(int V)
{
    V = V <= 0 ? 1 : V;
    const LARGE_INTEGER R = GetQueryPerformanceCounter();
    stb_srand(R.QuadPart);
    return (unsigned long)(stb_frand() * V);
}

static void GenerateNames(const int NamesCountInput, const int WordCountInput)
{
    static LARGE_INTEGER Start;
    static LARGE_INTEGER End;
    
    printf("\n");
    
    Start = GetQueryPerformanceCounter();
    AllocBuffer(Generate.NamesBuffer);
    
    char NameBuffer[1024];
    for(int NamesCountIndex = 0; NamesCountIndex < NamesCountInput; ++NamesCountIndex)
    {
        memset(&NameBuffer[0], 0, 1024);
        char* NameBufferPtr = &NameBuffer[0];
        
        for(int WordCountIndex = 0; WordCountIndex < WordCountInput; ++WordCountIndex)
        {
            // assumption lists are actually valid here
            const int RandomFileIndex = (int)(GetRand(Generate.Lists.FileCount));
            const int RandomWordIndex = (int)(GetRand(Generate.Lists.Files[RandomFileIndex].BufferCount));
            
            char* RandomWord = Generate.Lists.Files[RandomFileIndex].Buffer[RandomWordIndex];
            memcpy(NameBufferPtr, RandomWord, strlen(RandomWord));
            NameBufferPtr += strlen(RandomWord);
            memcpy(NameBufferPtr, " \n", 2);
            NameBufferPtr += 1;
        }
        
        const int NameLength = strlen(&NameBuffer[0]);
        memcpy(Generate.NamesBuffer.Ptr, &NameBuffer[0], NameLength);
        IncrementBuffer(Generate.NamesBuffer, NameLength);
        
        printf("%s", &NameBuffer[0]);
    }
    
    End = GetQueryPerformanceCounter();
    double Time = GetSecondsElapsed(Start, End);
    printf("Took %f seconds to generate %d names with %d words each.\n", Time, NamesCountInput, WordCountInput);
}

static void GenerateNameFromStarting(const char* Word)
{
    char NameBuffer[255];
    memset(&NameBuffer[0], 0, 255);
    char* NameBufferPtr = &NameBuffer[0];
    
    const int Length = GetTableLengthForWord(Word);
    if(Length)
    {
        const int RandomWordID = (int)(GetRand(Length));
        const char* RandWord = GetWordFromDictionary(Word, RandomWordID);
        
        memcpy(NameBufferPtr, RandWord, strlen(RandWord));
        NameBufferPtr += strlen(RandWord);
        memcpy(NameBufferPtr, " \n", 2);
        NameBufferPtr += 1;
        
        const int NameLength = strlen(&NameBuffer[0]);
        memcpy(Generate.NamesBuffer.Ptr, &NameBuffer[0], NameLength);
        IncrementBuffer(Generate.NamesBuffer, NameLength);
        
        printf("%s", &NameBuffer[0]);
    }
}

static void ClosingPrompt(const char* Prompt)
{
    printf("%s\n", Prompt);
    Sleep(1000);
}

static char** SplitWordCamelCase(char* Word, int& SplitCount)
{
    int Count = 0;
    int CapsIndex[255];
    
    char* Ptr = Word;
    while(*Ptr)
    {
        if(IsUpperCase(*Ptr))
        {
            CapsIndex[SplitCount++] = Count;
        }
        ++Count;
        ++Ptr;
    }
    
    char** Result = (char**)VirtualAlloc(nullptr, SplitCount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    int ResultIndex = 0;
    
    for(int CharacterIndex = 0; CharacterIndex < SplitCount; ++CharacterIndex)
    {
        const int WordLength = (int)strlen(Word);
        const int Size = (CharacterIndex + 1 < SplitCount) ? CapsIndex[CharacterIndex + 1] - CapsIndex[CharacterIndex] :  WordLength - CapsIndex[CharacterIndex];
        
        Result[ResultIndex] = (char*)VirtualAlloc(nullptr, Size + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        memcpy(Result[ResultIndex], &Word[CapsIndex[CharacterIndex]], Size);
        memset(&Result[ResultIndex][Size + 1], 0, 1);
        
        ++ResultIndex;
    }
    
    return Result;
}

static void GenerateNamesFrom(const int NamesCountInput, const char** NamesRoot, const int NamesRootCount)
{
    static LARGE_INTEGER Start;
    static LARGE_INTEGER End;
    
    printf("\n");
    
    Start = GetQueryPerformanceCounter();
    AllocBuffer(Generate.NamesBuffer);
    
    char NameBuffer[1024];
    for(int NameCountIndex = 0; NameCountIndex < NamesCountInput; ++NameCountIndex)
    {
        memset(&NameBuffer[0], 0, 1024);
        char* NameBufferPtr = &NameBuffer[0];
        
        for(int NameRootIndex = 0; NameRootIndex < NamesRootCount; ++NameRootIndex)
        {
            const int TableLength = GetTableLengthForWord(NamesRoot[NameRootIndex]);
            const int Random = (int)(GetRand(TableLength));
            if(char* RandomWord = GetWordFromDictionary(NamesRoot[NameRootIndex], Random))
            {
                memcpy(NameBufferPtr, RandomWord, strlen(RandomWord));
                NameBufferPtr += strlen(RandomWord);
                memcpy(NameBufferPtr, " \n", 2);
                NameBufferPtr += 1;
            }
        }
        
        const int NameLength = strlen(&NameBuffer[0]);
        memcpy(Generate.NamesBuffer.Ptr, &NameBuffer[0], NameLength);
        IncrementBuffer(Generate.NamesBuffer, NameLength);
        
        printf("%s", &NameBuffer[0]);
    }
    
    End = GetQueryPerformanceCounter();
    double Time = GetSecondsElapsed(Start, End);
    printf("Took %f seconds to generate %d names from root.\n", Time, NamesCountInput);
}

static bool Prompt(const commands Command)
{
    switch(Command)
    {
        case C_Help:
        {
            printf("Commands: ");
            for(int CommandIndex = 0; CommandIndex < C_MAX; ++CommandIndex)
            {
                printf("%s, ", Generate.Commands[CommandIndex]);
            }
            printf("\nType command to start...\n\n");
            return true;
        } break;
        case C_Generate:
        {
            int NamesCountInput = 0;
            int WordCountInput = 0;
            
            printf("Enter the number of names to generate...\n");
            scanf("%d", &NamesCountInput);
            
            printf("Enter the number of words per name to generate...\n");
            scanf("%d", &WordCountInput);
            
            if(NamesCountInput > 0  && WordCountInput > 0)
            {
                GenerateNames(NamesCountInput, WordCountInput);
                printf("\n");
                
                return Prompt(C_Help);
            }
        } break;
        case C_GenerateFrom:
        {
            int NamesCountInput = 0;
            char Current[255];
            
            printf("Enter the current word you want to generate from...\n");
            scanf("%s", &Current[0]);
            
            printf("Enter the number of names to generate...\n");
            scanf("%d", &NamesCountInput);
            
            int Count = 0;
            if(char** R = SplitWordCamelCase(Current, Count))
            {
                if(Count <= 0)
                {
                    return Prompt(C_Help);
                }
                GenerateNamesFrom(NamesCountInput, (const char**)R, Count);
            }
            
            return Prompt(C_Help);
        } break;
        case C_Copy:
        {
            if(OpenClipboard(nullptr) && EmptyClipboard())
            {
                if(HGLOBAL Handle = GlobalAlloc(GMEM_MOVEABLE, UsedSizeBuffer(Generate.NamesBuffer)))
                {
                    LPTSTR Lock = (LPTSTR)GlobalLock(Handle);
                    memcpy(Lock, Generate.NamesBuffer.StartPtr, UsedSizeBuffer(Generate.NamesBuffer));
                    GlobalUnlock(Handle);
                    
                    HANDLE CopyResult = SetClipboardData(CF_TEXT, Handle);
                    GlobalFree(Handle);
                    
                    if(CopyResult != NULL)
                    {
                        CloseClipboard();
                        printf("Copied names to clipboard\n\n");
                        return Prompt(C_Help);
                    }
                }
                
                ClosingPrompt("Error allocating memory to copy to clipboard. Closing...");
                return false;
            }
            
            ClosingPrompt("Error creating clipboard. Closing...");
            return false;
            
        } break;
        case C_Close:
        {
            ClosingPrompt("Requested closing...");
            return false;
        } 
    }
    return false;
}

static bool LoadContent()
{
    if(const char* ContentDirectory = GetContentDirectory())
    {
        if(Generate.Directories = stb_readdir_recursive((char*)ContentDirectory, nullptr))
        {
            StorageStartPtr = (char**)VirtualAlloc(nullptr, StorageBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            StoragePtr = StorageStartPtr;
            
            InitDictionary();
            
            for(int DirectoryIndex = 0; DirectoryIndex < stb_arr_len(Generate.Directories); ++DirectoryIndex)
            {
                word_file File = {};
                File.Buffer = stb_stringfile(Generate.Directories[DirectoryIndex], &File.BufferCount);
                
                AddToDictionary(File.Buffer, File.BufferCount);
                
                // storing data like this results in slower lookups. 
                stb_arr_push(Generate.Lists.Files, File);
            }
            Generate.Lists.FileCount = stb_arr_len(Generate.Lists.Files);
        }
    }
    else
    {
        printf("Failed to get content directory.\n");
        return false;
    }
    
    return Generate.Lists.FileCount > 0;
}

static int Close()
{
    FreeBuffer(Generate.NamesBuffer);
    stb_arr_free(Generate.Lists.Files);
    stb_readdir_free(Generate.Directories);
    return 0;
}

int main(int argc, char** argv)
{
    if(LoadContent())
    {
        QueryPerformanceFrequency(&Generate.Frequency);
        
        bool bValidCommand = Prompt(C_Help);
        while(bValidCommand)
        {
            char Buffer[255];
            scanf("%s", &Buffer[0]);
            
            bool bFoundValidCommand = false;
            for(int CommandIndex = 0; CommandIndex < C_MAX; ++CommandIndex)
            {
                if(char* Ptr = strstr(&Buffer[0], Generate.Commands[CommandIndex]))
                {
                    bFoundValidCommand = true;
                    bValidCommand = Prompt((commands)CommandIndex);
                    break;
                }
            }
            
            if(!bFoundValidCommand)
            {
                bValidCommand = Prompt(C_Help);
            }
        }
    }
    
    return Close();
}
