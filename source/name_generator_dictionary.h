/* date = September 8th 2020 2:56 pm */
#if !defined(NAME_GENERATOR_DICTIONARY_H)

struct page
{
    char ID;
    buffer* Population;
};

struct dictionary
{
    page* Tables;
};

static dictionary Dictionary;

static void InitDictionary()
{
    Dictionary.Tables = (page*)VirtualAlloc(nullptr, sizeof(page) * 26, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    for(int Index = 0; Index < 26; ++Index)
    {
        Dictionary.Tables[Index].ID = (char)(Index + 97);
    }
}

static bool IsUpperCase(const char Character)
{
    return (int)Character >= 65 && (int)Character <= 90;
}

static bool IsLowerCase(const char Character)
{
    return (int)Character >= 97 && Character <= 122;
}

static bool IsValidCharacter(const char Character)
{
    return IsUpperCase(Character) || IsLowerCase(Character);
}

static char ToLower(const char Upper)
{
    if(IsUpperCase(Upper))
    {
        return Upper ^ 0x20;
    }
    return Upper;
}

static void AddToDictionary(char** List, const int ListLength)
{
    for(int Index = 0; Index < ListLength; ++Index)
    {
        const char ID = ToLower(List[Index][0]);
        if(IsValidCharacter(ID))
        {
            buffer B = {nullptr, nullptr, (int)strlen(List[Index])};
            AllocBuffer(B);
            memcpy(B.Ptr, List[Index], B.Size);
            
            stb_arr_push(Dictionary.Tables[(int)ID - 97].Population, B);
        }
    }
}

static int GetTableLengthForWord(const char* Word)
{
    const char ID = ToLower(Word[0]);
    if(IsValidCharacter(ID))
    {
        return stb_arr_len(Dictionary.Tables[(int)ID - 97].Population);
    }
    return 0;
}

static char* GetWordFromDictionary(const char* CurrentWord, const int RandomNumber)
{
    const char ID = ToLower(CurrentWord[0]);
    if(IsValidCharacter(ID))
    {
        return Dictionary.Tables[(int)ID - 97].Population[RandomNumber].StartPtr;
    }
    return nullptr;
}



#define NAME_GENERATOR_DICTIONARY_H
#endif //NAME_GENERATOR_DICTIONARY_H
