# NobodyDLSS: findings

Game facts (paths, engine, console variable layout, the game's own upscalers) are in
NobodyTweaks' `docs/FINDINGS.md`. This add-on is DoubleExposureDLSS 1.2.1 (UE 5.2.1) ported to
Nobody Wants to Die (UE 5.3, build 14922697); DoubleExposureDLSS's findings are kept in
`FINDINGS-DoubleExposure.md` for comparison.

## Port (0.0.1, 2026-11-01)

- `cvars.hpp` is NobodyTweaks' (FindConsoleVariable +0xb8, SetBy 0x09/0x0A, accepts a console
  manager whose vtable another add-on copied).
- `r.ScreenPercentage` (DLSS render resolution) is set on the game thread from a hooked
  CallAllConsoleVariableSinks, chained with NobodyTweaks' hook (whichever loads second copies
  the first's vtable; both modules are pinned). DoubleExposureDLSS set it from ReShade's thread
  and copied it to the render thread by hand.
- Mapping build: TSR shader ids are placeholders and `kTsrMapped=false` keeps DLSS and frame
  generation off. F10 writes every compute pass of two frames to `NobodyDLSS-frame-N.txt`.

## 0.0.1 crash and 0.0.2 (2026-11-01)

0.0.1's capture crashed the game on the RHI thread: a descriptor slot still named a resource UE
had freed, and get_resource_desc read freed memory (AV at 0xffffffffffffffff). 0.0.2 guards the
capture's resource and buffer reads with SEH. build.ps1 writes `bin\obj\NobodyDLSS.map` (the
system dbghelp can't read the PDB): look crash RVAs up there.

## UE 5.3 TSR (capture 2026-11-01, TSR 67% at 3840x2160: render 2560x1440)

| pass | id | queue | bindings |
|---|---|---|---|
| ClearPrevTextures | 42e09aac (same as 5.2) | async compute | u0-u2 r32uint |
| (ForwardScatterDepth?) | 52618d50 | async compute | t0 r32uint -> u0 r32uint |
| DilateVelocity | 87b9be71 | async compute | t0 depth r32g8x24, t1 velocity rgba16unorm -> u0 rg16unorm, u1 r16f, ... |
| DecimateHistory | 090c206f | async compute | t0 = DilateVelocity u0; u0 rgb10a2, u1 rgba8, u2 rg16unorm, u3 rg8, u4 r8 |
| RejectShading | 161118cb | graphics | t0 scene colour rgba16f, t2 translucency rgba16f, t3-t5 Decimate outputs |
| SpatialAntiAliasing | 93b776bc | graphics | |
| FilterAntiAliasing | 1c0a8a60 | graphics | t0 r8uint -> u0 r8uint |
| UpdateHistory | fa4cc3c5 | graphics | t0 scene colour, t1 translucency, t3 = Decimate u2, t8 previous history -> u0 output 3840x2160 r11g11b10f, u1 1920x1080 half-res, u2 r8unorm |

No ResolveHistory: the history is at 100% and UpdateHistory's u0 is the output (two textures
alternate). UpdateHistory's param 2 has 5.2 ResolveHistory's layout: extent 0, min 32, size 48,
jitter 256, motion vector scale 272. The View UB is DilateVelocity's param 3.

View UB offsets are UE 5.6's, not 5.2's: TranslatedWorldToClip 0, RelativeWorldToClip 64,
ClipToRelativeWorld 128, TranslatedWorldToView 192, ViewToTranslatedWorld 256, ViewToClip 448
(jittered; [2][2]=0 infinite far, near 1), ViewToClipNoAA 512, ClipToView 576.

0.1.0: DLSS runs at UpdateHistory (the game's pass first, then DLSS overwrites u0). The chain
crosses queues: DilateVelocity's link is passed on under DecimateHistory's u2 and found at
UpdateHistory's t3. Translucency is still blended by TSR (UpdateHistory t1), so compose.hlsl
carries over.

## DLSS crashes and the game's own NGX (2026-11-01)

- 0.1.0: DLSS feature created, first evaluation failed in NGX (`NvAPI_D3D12_LaunchCuKernelChain
  failed - nvapi status -1`, 0xbad00002), which broke the command list: UE aborted at Close
  (E_INVALIDARG).
- 0.1.1 (own depth/output copies): access violation in D3D12Core inside NGX's CreateFeature.
- The game's DLSS plugin starts NGX at every launch (its `Saved\Logs\NGX\NGX_<time>` folder),
  even with TSR chosen. With `[SystemSettings] r.NGX.Enable=0` in the user's Engine.ini (backup
  `Engine.ini.bak-nobodydlss`) DLSS ran: 120 of 120 frames upscaled, 1920x1080 -> 3840x2160.
  0.1.2 writes that line itself when DLSS or DLSS frame generation is wanted, and doesn't start
  NGX or Streamline while `r.NGX.Enable` is 1 this session. The own depth/output copies stay
  (unproven either way; cheap).
- 0.1.1 then crashed when a video ended: the game's Electra video decoder destroys its own device
  and the add-on's destroy_device handlers cleaned up for any device. 0.1.2 only cleans up for the
  game's device.
- A crash 10 s after launch (before DLSS was on) in `FSlateRHIResourceManager::
  FindOrCreateDynamicTextureResource` via the FSR 3 plugin's Slate renderer, around NobodyTweaks'
  HDR switch: same signature as NobodyTweaks 0.3.2's crash, not this add-on.

## Frame generation (2026-11-01)

- The game's FSR 3 plugin wraps the swapchain while `r.FidelityFX.FI.OverrideSwapChainDX12=1`
  (its default). 0.2.0 writes it as 0 to Engine.ini when frame generation is picked and doesn't
  hook the swapchain while it's 1. UE rewrote Engine.ini at exit and dropped the line; 0.2.1
  writes its lines again at unload (and they stayed).
- The three early device creations at launch read it as 1 (Engine.ini's [SystemSettings] isn't
  applied yet); the game's real device reads 0.
- 0.2.1 in game (user, 2026-11-01): **DLSS Frame Generation 2x works**. Streamline 2.14 on the
  game's swapchain, View buffer check passed (near 1.0), first frame tagged at submission
  (kTagAtRecord=false works for 5.3), clean Streamline shutdown at exit, no crash.

## FSR (2026-11-01, 0.2.1)

User tested FSR upscaling and FSR frame generation; log clean: FSR 3.1.6 upscaler 2228x1253 ->
3840x2160 (camera near 1.0, vertical FOV 58.7 degrees from ViewToClip at 448), FSR FG 3.1.6 2x in
HDR on the game's swapchain (r.FidelityFX.FI.OverrideSwapChainDX12=0), all frames tagged, HUD
told apart with the HUD-mask composite (and the pre-HUD picture in some scenes), ordered shutdown
(context released, then the swapchain), no crash. Inputs are submitted on one thread and presented
on another in UE 5.3; the handover at submission works.

## To find out

- ClipToPrevClip offset (searched at run time; logged).
- Velocity encoding (5.2: square-root "gamma" decode, the default; 5.6: linear).
- Whether 5.3 submits on its own thread (`kTagAtRecord`).
- Streamline's interposer next to the game's FSR 3 frame interpolation swap chain
  (`r.FidelityFX.FI.OverrideSwapChainDX12=1`).
- NobodyTweaks 0.4.0 removed its FSR 3 frame generation option, so NobodyDLSS's is the only one.
