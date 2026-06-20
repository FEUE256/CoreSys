#pragma once

#include <core/efi.h>
#include <core/efi_lib.h>

static EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
static EFI_SIMPLE_POINTER_PROTOCOL *Spp;

typedef struct {
    UINTN X;
    UINTN Y;
} MOUSE_POS;

typedef struct {
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
    UINT8 Reserved;
} PIXEL;

static MOUSE_POS Mouse = {0, 0};

void Rect(UINTN x, UINTN y, UINTN w, UINTN h, UINT8 r, UINT8 g, UINT8 b)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL c = { b, g, r, 0 };

    Gop->Blt(
        Gop,
        &c,
        EfiBltVideoFill,
        0, 0,
        x, y,
        w, h,
        0
    );
}

void DrawText(UINTN x, UINTN y, CHAR16 *text)
{
    // assumes you already have EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
    gST->ConOut->SetCursorPosition(gST->ConOut, x, y);
    gST->ConOut->OutputString(gST->ConOut, text);
}

void DrawMouse()
{
    Rect(Mouse.X, Mouse.Y, 6, 6, 255, 255, 255);
}

void UpdateMouse()
{
    if (!Spp) return;

    EFI_SIMPLE_POINTER_STATE state;
    if (EFI_ERROR(Spp->GetState(Spp, &state)))
        return;

    // SPP is RELATIVE movement (IMPORTANT FIX)
    if (state.RelativeMovementX != 0)
        Mouse.X += (INTN)state.RelativeMovementX / 1000;

    if (state.RelativeMovementY != 0)
        Mouse.Y += (INTN)state.RelativeMovementY / 1000;

    // clamp
    UINTN sw = Gop->Mode->Info->HorizontalResolution;
    UINTN sh = Gop->Mode->Info->VerticalResolution;

    if (Mouse.X > sw) Mouse.X = sw;
    if (Mouse.Y > sh) Mouse.Y = sh;
}

void RectUI(UINTN x, UINTN y, UINTN w, UINTN h, UINT8 r, UINT8 g, UINT8 b)
{
    Rect(x, y, w, h, r, g, b);
}

void DrawDesktop()
{
    UINTN sw = Gop->Mode->Info->HorizontalResolution;
    UINTN sh = Gop->Mode->Info->VerticalResolution;

    Rect(0, 0, sw, sh, 20, 35, 70);

    Rect(0, sh - 35, sw, 35, 40, 40, 40);

    Rect(180, 100, 700, 450, 220, 220, 220);

    Rect(180, 100, 700, 35, 40, 90, 200);

    Rect(840, 108, 25, 18, 200, 50, 50);

    Rect(430, 320, 180, 50, 60, 180, 80);

    Rect(350, 220, 350, 40, 255, 255, 255);

    DrawText(360, 230, L"CoreSys UI - UEFI Desktop");
    DrawText(360, 260, L"Click buttons or move mouse");
    DrawText(360, 290, L"Press ESC to exit");
}

void Redraw()
{
    DrawDesktop();
    DrawMouse();
}

EFI_STATUS gmain()
{
    clear_screen();

    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GUID sppGuid = EFI_SIMPLE_POINTER_PROTOCOL_GUID;

    EFI_STATUS Status;

    Status = gST->BootServices->LocateProtocol(&gopGuid, NULL, (VOID**)&Gop);
    if (EFI_ERROR(Status)) return Status;

    Status = gST->BootServices->LocateProtocol(&sppGuid, NULL, (VOID**)&Spp);
    if (EFI_ERROR(Status)) return Status;

    Redraw();

    EFI_INPUT_KEY key;

    while (1)
    {
        UINTN index;

        gST->BootServices->WaitForEvent(
            1,
            &gST->ConIn->WaitForKey,
            &index
        );

        if (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &key)))
        {
            if (key.ScanCode == SCANCODE_ESC)
                break;

            if (key.UnicodeChar == ' ')
                Redraw();
        }

        UpdateMouse();
        Redraw();
    }

    return EFI_SUCCESS;
}
