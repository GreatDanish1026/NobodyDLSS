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

## To find out

- UE 5.3 TSR passes and their shader ids (capture with TSR chosen in the game's menu).
- View uniform buffer offsets (5.2: TranslatedWorldToClip 0, RelativeWorldToClip 128,
  ViewToTranslatedWorld 320, ViewToClip 512, ViewToClipNoAA 576) and ClipToPrevClip.
- Velocity encoding (5.2: square-root "gamma" decode; 5.6: linear).
- Whether separate translucency is still blended by TSR (5.2: UpdateHistory t1).
- Whether 5.3 submits on its own thread (`kTagAtRecord`).
- Streamline's interposer next to the game's FSR 3 frame interpolation swap chain
  (`r.FidelityFX.FI.OverrideSwapChainDX12=1`).
