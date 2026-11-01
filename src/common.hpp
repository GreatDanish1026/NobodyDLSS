// Shared between the capture tool (main.cpp), the TSR hook (upscaler.cpp) and the NGX wrapper
// (dlss.cpp).
#pragma once

#ifndef ImTextureID
#define ImTextureID ImU64
#endif
#include <imgui.h>
#include <reshade.hpp>

#include <cstdint>
#include <string>

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace rd {

// NobodyDLSS.log next to the game exe.
void Log(const char* fmt, ...);

// The folder of the game exe, with a trailing backslash.
const std::wstring& ExeDir();

// FNV-1a of a compute pipeline's shader bytecode (0 if unknown): stable across runs.
uint32_t PipeId(reshade::api::pipeline pipe);

// TSR's compute shaders in this game (Nobody Wants to Die build 14922697, UE 5.3.2; capture
// 2026-11-01, docs/FINDINGS.md), in the order they run. The first four run on the async compute
// queue, the rest on the graphics queue. UE 5.3 has no ResolveHistory: UpdateHistory writes the
// 3840x2160 output itself (history at 100%).
constexpr bool kTsrMapped = true;
// Frame generation's inputs go to their frame when the game submits them (false; UE 5.2 submits
// and presents on the same thread) or when they're recorded (true, ReunionDLSS: UE 5.6 submits on
// a thread of its own). To check for UE 5.3. framegen.cpp is shared with ReunionDLSS.
constexpr bool kTagAtRecord = false;
constexpr uint32_t kTsrClearPrevTextures = 0x42e09aac;    // same id as UE 5.2
constexpr uint32_t kTsrForwardScatterDepth = 0x52618d50;  // r32uint t0 -> u0 (name from 5.2's order)
constexpr uint32_t kTsrDilateVelocity = 0x87b9be71;       // t0 depth, t1 velocity -> u0 dilated velocity
constexpr uint32_t kTsrDecimateHistory = 0x090c206f;      // t0 = DilateVelocity's u0; u2 -> UpdateHistory t3
constexpr uint32_t kTsrRejectShading = 0x161118cb;        // graphics queue from here
constexpr uint32_t kTsrSpatialAntiAliasing = 0x93b776bc;
constexpr uint32_t kTsrFilterAntiAliasing = 0x1c0a8a60;
constexpr uint32_t kTsrUpdateHistory = 0xfa4cc3c5;        // t0 scene colour, t1 translucency, t3 link -> u0 output
constexpr uint32_t kTsrResolveHistory = 0xfffffff8;       // none in UE 5.3 (placeholder, never matches)

// The game's own DLSS plugin starts NVIDIA's NGX at launch (r.NGX.Enable=1). A second NGX user in the
// process (our DLSS, Streamline's frame generation) then crashed inside NGX (2026-11-01).
// GameNgxRunning: r.NGX.Enable is on this session (DLSS and Streamline aren't started then).
// TurnGameNgxOff: writes r.NGX.Enable=0 to the game's Engine.ini (read at the next launch).
bool GameNgxRunning();
void TurnGameNgxOff();
extern const char* const kGameNgxRestart;  // the message shown while it's running

// upscaler.cpp
void RegisterUpscaler();
void UnregisterUpscaler();
// Overlay: the Settings tab and the Status tab.
void DrawUpscalerSettings();
void DrawUpscalerStatus();
void DrawUpscalerAdvanced();
// One line for the top of the overlay; *warn when it's a problem.
std::string UpscalerHeadline(bool* warn);

// Overlay helpers (main.cpp), in the Tweaks add-ons' style.
float ControlWidth();
void Help(const char* text);  // a "(?)" after the last item, with <text> as its tooltip
extern const ImVec4 kWarn;
// Puts the game's pipeline, root signatures, descriptor heaps and bindings back on <cl> after
// the add-on recorded work of its own on it.
void RestoreListState(reshade::api::command_list* cl);

// framegen.cpp: DLSS Frame Generation through Streamline.
void RegisterFrameGen();
void UnregisterFrameGen();
void DrawFrameGenSettings();
void DrawFrameGenStatus();
std::string FrameGenHeadline(bool* warn);
// The View uniform buffer (first 16 KB) of this frame's main view, with View.ClipToPrevClip's offset.
void FrameGenSetCamera(const float* view, int clipToPrevClipOffset);
// True if a TSR output of this size is the picture on screen (the back buffer's size).
bool FrameGenIsScreen(unsigned width, unsigned height);
std::string FrameGenDiag();  // diagnostics for the upscaler's diag line
// This frame's depth (UE's, state depth read + shader resource) and our motion vectors (render
// pixels, previous minus current; state non-pixel shader resource), tagged on <list>.
void FrameGenTag(ID3D12GraphicsCommandList* list, ID3D12Resource* depth, ID3D12Resource* motion, unsigned renderWidth,
                 unsigned renderHeight, float jitterX, float jitterY, bool reset);

}  // namespace rd
