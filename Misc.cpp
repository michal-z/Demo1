namespace Misc
{

static eastl::vector<uint8_t>
LoadFile(const char* FileName)
{
    FILE* File = fopen(FileName, "rb");
    assert(File);
    fseek(File, 0, SEEK_END);
    long Size = ftell(File);
    assert(Size != -1);
    eastl::vector<uint8_t> Content(Size);
    fseek(File, 0, SEEK_SET);
    fread(&Content[0], 1, Content.size(), File);
    fclose(File);
    return Content;
}

static double
GetTime()
{
    static LARGE_INTEGER StartCounter;
    static LARGE_INTEGER Frequency;
    if (StartCounter.QuadPart == 0)
    {
        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartCounter);
    }
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    return (Counter.QuadPart - StartCounter.QuadPart) / (double)Frequency.QuadPart;
}

// returns [0.0f, 1.0f)
static inline float
Randomf()
{
	unsigned Exponent = 127;
	unsigned Significand = (unsigned)(rand() & 0x7fff); // get 15 random bits
	unsigned Result = (Exponent << 23) | (Significand << 8);
	return *(float*)&Result - 1.0f;
}

// returns [begin, end)
static inline float
Randomf(float Begin, float End)
{
	assert(Begin < End);
	return Begin + (End - Begin) * Randomf();
}

} // namespace Misc
// vim: set ts=4 sw=4 expandtab:
