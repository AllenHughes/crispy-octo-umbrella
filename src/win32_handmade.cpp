#include <math.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#include "handmade.h"
#include "handmade.cpp"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

global_variable bool32 Running;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


// These two stub functions are used incase XInput is unavailable. So we can avaoid errors
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED); 
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED); 
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID pcGuidDevice,LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal debug_read_file_result
DEBUGPlatformReadEntireFile(char *Filename)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUint64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
            }
        }
        else
        {
        }
        CloseHandle(FileHandle);
    }
    else
    {
    }
    
    return(Result);
}
internal void
DEBUGPlatformFreeFileMemory(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}
internal bool32
DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory)
{
    bool32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            DWORD BytesWritten;
            if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
            {
                Result = (BytesWritten == MemorySize);
            }
            else
            {
            }
        }
        else
        {
        }
        CloseHandle(FileHandle);
    }
    else
    {
    }
    
    return(Result);
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(Allen): Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
 
    if(DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
      
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
          
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                            
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                    }
                    else
                    {
                    }
                }
            }
            else
            {
            }
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;

            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {

            }
        }
        else
        {
          
        }
    } 
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}
 
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
  
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
  
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;  
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer Buffer, HDC DeviceContext,
                           int WindowWidth, int WindowHeight, int X, int Y, int Width, int Height)
{
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,                
                  0, 0, Buffer.Width, Buffer.Height,
                  Buffer.Memory,
                  &Buffer.Info,
                  DIB_RGB_COLORS, SRCCOPY);  
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
  
    switch(Message)
    {
    case WM_SIZE:
    {
        
    } break;
      
    case WM_DESTROY:
    {
        Running = false;
    } break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
    }break;

    case WM_CLOSE:
    {
        Running = false;
        OutputDebugStringA("WM_CLOSE\n");       
    }break;
      
    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n"); 
    }break;
      
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window,&Paint);
        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;          
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;

        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(GlobalBackbuffer, DeviceContext, Dimension.Width,
                                   Dimension.Height, X, Y, Height, Width);
        EndPaint(Window, &Paint);
    }break;

    default:
    {
        Result = DefWindowProcA(Window, Message, WParam, LParam);
    }break;
      
    }
    return(Result);
}

internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;            
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            
            *DestSample++ = 0;
        }

        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;            
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            
            *DestSample++ = 0;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput,
                     DWORD ByteToLock, DWORD BytesToWrite,
                     game_sound_output_buffer *SoundBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
                
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SoundBuffer->Samples;
        for(DWORD SampleIndex = 0;            
            SampleIndex < Region1SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
                        
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample= (int16 *)Region2;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region2SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}
internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool IsDown)
{
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

internal void
Win32ProcessPendingMessages(game_controller_input *KeyboardController)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        if(Message.message == WM_QUIT)
        {
            Running = false;
        }
        switch(Message.message)
        {
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = (uint32)Message.wParam;
            bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
            bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);

            if(WasDown != IsDown)
            {
                if(VKCode == 'W') {
                }
                else if(VKCode == 'A')
                {
                }
                else if(VKCode == 'S')
                {
                }
                else if(VKCode == 'D')
                {
                }
                else if(VKCode == 'Q')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                }
                else if(VKCode == 'E')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                }
                else if(VKCode == VK_UP)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Up, IsDown);
                }
                else if(VKCode == VK_LEFT)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Left, IsDown);
                }
                else if(VKCode == VK_DOWN)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Down, IsDown);
                }
                else if(VKCode == VK_RIGHT)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Up, IsDown);
                }
                else if(VKCode == VK_ESCAPE)
                {
                    Running = false;
                }
                else if(VKCode == VK_SPACE)
                {
                }           
            }
            // Close window with alt+F4 since we take controll of SYSKEYDOWN
            bool32 AltKeyWasDown = Message.lParam & (1 << 29); 
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                Running = false;
            }
        } break;
        default:
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        } break;
        }
    }
}
internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState,
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64 LastCycleCounter = __rdtsc();    
    int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass. hInstance = Instance;
    //  WindowClass.hIcon;
    WindowClass.lpszClassName = "GameWindowClass";

    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(0,
                                      WindowClass.lpszClassName,
                                      "Game Window",
                                      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      0,
                                      0,
                                      Instance,
                                      0);
        if(Window)
        {

            HDC DeviceContext = GetDC(Window);
          
            int XOffset = 0;
            int YOffset = 0;

            win32_sound_output SoundOutput = {};

            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.BytesPerSample = sizeof(int16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond/15;
            
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            
            Win32ClearBuffer(&SoundOutput);
            
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            Running = true;

            int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
                                                   MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes((uint64)2);
#else
            LPVOID BaseAddress = 0;
#endif
            
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Gigabytes(1);
            uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize,
                                                       MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorage = (uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

            
            if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {

                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];
            
                LARGE_INTEGER LastCounter;
                QueryPerformanceCounter(&LastCounter);
                while(Running)
                {

                    game_controller_input *KeyboardController = &NewInput->Controllers[0];
                    game_controller_input ZeroController = {};
                    *KeyboardController = ZeroController;

                    Win32ProcessPendingMessages(KeyboardController);
                    
                    DWORD MaxControllerCount = XUSER_MAX_COUNT;
                    if (MaxControllerCount > ArrayCount(NewInput->Controllers))
                    {
                        MaxControllerCount =  ArrayCount(NewInput->Controllers);
                    }
                
                    for(DWORD ControllerIndex = 0;
                        ControllerIndex < MaxControllerCount;
                        ++ControllerIndex)
                    {
                        game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
                        game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];

                        XINPUT_STATE ControllerState;                  
                        //  dwResult = XInputGetState(ControllerIndex, &ControllerState);
                  
                        if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                        {
                            XINPUT_GAMEPAD *Pad =  &ControllerState.Gamepad;

                            //D-Pad
                            bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                            bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                            bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                            bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

                            NewController->IsAnalog = true;
                            NewController->StartX = OldController->EndX;
                            NewController->StartY = OldController->EndY;

                            real32 x;
                            if(Pad->sThumbLX < 0)
                            {
                                x = (real32)Pad->sThumbLX / 32768.0f;
                            }
                            else
                            {
                                x = (real32)Pad->sThumbLX / 32767.0f;
                            }
                            NewController->MinX = NewController->MaxX = NewController->EndX = x;

                            real32 y;
                            if(Pad->sThumbLX < 0)
                            {
                                y = (real32)Pad->sThumbLY / 32768.0f;
                            }
                            else
                            {
                                y = (real32)Pad->sThumbLY / 32767.0f;
                            }
                            NewController->MinY = NewController->MaxY = NewController->EndY = y;
                        
                            //Thumbsticks
                            int16 StickX = Pad->sThumbLX / 32;
                            int16 StickY = Pad->sThumbLY;



                            Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                            &OldController->Down,
                                                            XINPUT_GAMEPAD_A,
                                                            &NewController->Down);
                        
                            Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                            &OldController->Right,
                                                            XINPUT_GAMEPAD_B,
                                                            &NewController->Right);
                        
                            Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                            &OldController->Left,
                                                            XINPUT_GAMEPAD_X,
                                                            &NewController->Left);
                        
                            Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                            &OldController->Up,
                                                            XINPUT_GAMEPAD_Y,
                                                            &NewController->Up);
                        
                            Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                            &OldController->LeftShoulder,
                                                            XINPUT_GAMEPAD_LEFT_SHOULDER,
                                                            &NewController->LeftShoulder);
                        
                            Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                            &OldController->RightShoulder,
                                                            XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                                            &NewController->RightShoulder);
                        
                            // bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                            // bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);             
                        }
                        else
                        {

                        }
                    }
                    DWORD ByteToLock = 0;
                    DWORD TargetCursor;
                    DWORD BytesToWrite = 0;
                    DWORD PlayCursor;
                    DWORD WriteCursor;
                    bool32 SoundIsValid = false;
                    if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor,
                                                                           &WriteCursor)))
                    {
                        ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) %
                                      SoundOutput.SecondaryBufferSize);
                    
                        TargetCursor = ((PlayCursor +
                                         (SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) %
                                        SoundOutput.SecondaryBufferSize);
                    

                        if(ByteToLock > TargetCursor)
                        {
                            BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                            BytesToWrite += TargetCursor;
                        }
                        else
                        {
                            BytesToWrite = TargetCursor  - ByteToLock; 
                        }
                    
                        SoundIsValid = true;
                    }



                    game_sound_output_buffer SoundBuffer = {};
                    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                    SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                    SoundBuffer.Samples = Samples;               

                    game_offscreen_buffer  Buffer = {};
                    Buffer.Memory = GlobalBackbuffer.Memory;
                    Buffer.Width = GlobalBackbuffer.Width;
                    Buffer.Height = GlobalBackbuffer.Height;
                    Buffer.Pitch = GlobalBackbuffer.Pitch;
                                                
                    GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

                    /*
                     * DirectSound Setup
                     */

                    if(SoundIsValid)
                    {
                        Win32FillSoundBuffer(&SoundOutput,
                                             ByteToLock, BytesToWrite,
                                             &SoundBuffer);
                    }
                
                    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                    Win32DisplayBufferInWindow(GlobalBackbuffer, DeviceContext,
                                               Dimension.Width, Dimension.Height,
                                               0, 0, Dimension.Width, Dimension.Height);               

                    ++XOffset;
                    YOffset += 2;

                    int64 EndCycleCounter = __rdtsc();
                                    
                    LARGE_INTEGER EndCounter;
                    QueryPerformanceCounter(&EndCounter);

                    uint64 CyclesElapsed = EndCycleCounter - LastCycleCounter;
                    int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    real64 MSPerFrame = (((1000.0f*(real64)CounterElapsed) / (real64)PerfCountFrequency));
                    real32 FPS = (real32)(PerfCountFrequency/CounterElapsed);
                    real32 MCPF =  (real32)(CyclesElapsed / (1000 * 1000));

                    // char PrintBuffer[256];
                    // wsprintf(PrintBuffer, "%dms/f, %df/s,  %dmc/f\n", MSPerFrame, FPS, MCPF);
                    // OutputDebugString(PrintBuffer);

                    LastCycleCounter = EndCycleCounter; 
                    LastCounter = EndCounter;

                    // Need to write swap(NewInput, OldInput);
                    game_input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                }
            } else
            {
            }
        }
    }
    return(0);
}
