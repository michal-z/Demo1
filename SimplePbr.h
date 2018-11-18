namespace SimplePbr
{

static void Update(double Time,
                   float DeltaTime);
static void Initialize();
static void Shutdown();

} // SimplePbr

namespace Dx
{

static ID3D12Device* GDevice;
static ID3D12CommandQueue* GCmdQueue;
static ID3D12CommandAllocator* GCmdAlloc[2];
static ID3D12GraphicsCommandList* GCmdList;
static ID3D12Resource* GSwapBuffers[4];
static ID3D12Resource* GDepthBuffer;
static HWND GWindow;
static unsigned GResolution[2];
static unsigned GDescriptorSize;
static unsigned GDescriptorSizeRtv;
static unsigned GFrameIndex;
static unsigned GBackBufferIndex;
static eastl::vector<ID3D12Resource*> GIntermediateResources;

static void AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                unsigned Count,
                                D3D12_CPU_DESCRIPTOR_HANDLE& OutFirst);
static void AllocateGpuDescriptors(unsigned Count,
                                   D3D12_CPU_DESCRIPTOR_HANDLE& OutFirstCpu,
                                   D3D12_GPU_DESCRIPTOR_HANDLE& OutFirstGpu);
static D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptorsToGpu(unsigned Count,
                                                        D3D12_CPU_DESCRIPTOR_HANDLE Source);
static inline D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtv();
static inline void SetDescriptorHeap();
static void* AllocateGpuUploadMemory(unsigned Size,
                                     D3D12_GPU_VIRTUAL_ADDRESS& OutGpuAddress);
static void Initialize(HWND Window);
static void Shutdown();
static void PresentFrame();
static void WaitForGpu();

} // namespace Dx

namespace Gui
{

static void Initialize();
static void Render();
static void Shutdown();

} // namespace Gui

namespace Misc
{

static eastl::vector<uint8_t> LoadFile(const char* FileName);
static double GetTime();
static inline float Randomf();
static inline float Randomf(float Begin, float End);

} // namespace Misc
// vim: set ts=4 sw=4 expandtab:
