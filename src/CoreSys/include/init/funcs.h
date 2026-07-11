#pragma once

#include <core/efi.h>
#include <core/efi_lib.h>
#include <GUI/main.h>
#include <crypto/main.h>
#include <project/codes.h>

#define DEFAULT_FG_COLOR EFI_YELLOW
#define DEFAULT_BG_COLOR EFI_BLUE

#define HIGHLIGHT_FG_COLOR EFI_BLUE
#define HIGHLIGHT_BG_COLOR EFI_CYAN

// Replace dynamic macros with compile-time constant structs

static const EFI_GRAPHICS_OUTPUT_BLT_PIXEL EFI_PIXEL_BLACK = {0x00, 0x00, 0x00, 0x00};
static const EFI_GRAPHICS_OUTPUT_BLT_PIXEL EFI_PIXEL_RED   = {0x00, 0x00, 0xFF, 0x00};
static const EFI_GRAPHICS_OUTPUT_BLT_PIXEL px_BLUE = {0, 0, 0xFF, 0};

EFI_GRAPHICS_OUTPUT_BLT_PIXEL cursor_buffer[64] = {

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK,
    EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,   EFI_PIXEL_RED,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK,
    EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,   EFI_PIXEL_RED,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK,
    EFI_PIXEL_RED,   EFI_PIXEL_RED,   EFI_PIXEL_RED,   EFI_PIXEL_RED,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_BLACK,
    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,
    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,
    EFI_PIXEL_RED,   EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_BLACK,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,
    EFI_PIXEL_RED,   EFI_PIXEL_RED,   EFI_PIXEL_BLACK, EFI_PIXEL_BLACK,

    EFI_PIXEL_BLACK, EFI_PIXEL_BLACK, EFI_PIXEL_RED,   EFI_PIXEL_RED,
    EFI_PIXEL_RED,   EFI_PIXEL_RED,   EFI_PIXEL_BLACK, EFI_PIXEL_BLACK,
};

// Buffer to save Framebuffer data at cursor position
EFI_GRAPHICS_OUTPUT_BLT_PIXEL save_buffer[8*8] = {0};

// ==========================
// Get Memory Map from UEFI
// ==========================
EFI_STATUS get_memory_map(Memory_Map_Info *mmap) { 
    memset(mmap, 0, sizeof *mmap);  // Ensure input parm is initialized

    // Get initial memory map size (send 0 for map size)
    EFI_STATUS status = EFI_SUCCESS;
    status = bs->GetMemoryMap(&mmap->size,
                              mmap->map,
                              &mmap->key,
                              &mmap->desc_size,
                              &mmap->desc_version);

    if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
        error(0, u"Could not get initial memory map size.\r\n");
        return status;
    }

    // Allocate buffer for actual memory map for size in mmap->size;
    //   need to allocate enough space for an additional memory descriptor or 2 in the map due to
    //   this allocation itself.
    mmap->size += mmap->desc_size * 2;  
    status = bs->AllocatePool(EfiLoaderData, mmap->size,(VOID **)&mmap->map);
    if (EFI_ERROR(status)) {
        error(status, u"Could not allocate buffer for memory map '%s'\r\n");
        return status;
    }

    // Call get memory map again to get the actual memory map now that the buffer is the correct
    //   size
    status = bs->GetMemoryMap(&mmap->size,
                              mmap->map,
                              &mmap->key,
                              &mmap->desc_size,
                              &mmap->desc_version);
    if (EFI_ERROR(status)) {
        error(status, u"Could not get UEFI memory map! :(\r\n");
        return status;
    }

    return EFI_SUCCESS;
}

// ====================
// Set Text Mode
// ====================
EFI_STATUS set_text_mode(void) {
    // Store found Text mode info
    typedef struct {
	INTN  mode;
        UINTN cols;
        UINTN rows;
    } Text_Mode_Info;

    Text_Mode_Info text_modes[20];

    UINTN mode_index = 0;   // Current mode within entire menu of text mode choices 

    // Overall screen loop
    while (true) {
        cout->ClearScreen(cout);

        // Get current text mode info
        UINTN max_cols = 0, max_rows = 0;
        cout->QueryMode(cout, cout->Mode->Mode, &max_cols, &max_rows);

        printf_c16(u"Text mode information:\r\n"
                   u"Max Mode: %d\r\n"
                   u"Current Mode: %d\r\n"
                   u"Attribute: %x\r\n" 
                   u"CursorColumn: %d\r\n"
                   u"CursorRow: %d\r\n"
                   u"CursorVisible: %d\r\n"
                   u"Columns: %d\r\n"
                   u"Rows: %d\r\n\r\n",
                   cout->Mode->MaxMode,
                   cout->Mode->Mode,
                   cout->Mode->Attribute,
                   cout->Mode->CursorColumn,
                   cout->Mode->CursorRow,
                   cout->Mode->CursorVisible,
                   max_cols,
                   max_rows);

        printf_c16(u"Available text modes:\r\n");

        UINTN menu_top = cout->Mode->CursorRow;

        // Print keybinds at bottom of screen
        cout->SetCursorPosition(cout, 0, max_rows-3);
        printf_c16(u"Up/Down Arrow = Move Cursor\r\n"
               u"Enter = Select\r\n"
               u"Escape = Go Back");

        UINTN menu_bottom = max_rows-5;	// Stop above keybind text (0-based offset)

        // Get all valid text modes' info
	// NOTE: Max valid GOP mode is ModeMax-1 per UEFI spec
        UINT32 max = cout->Mode->MaxMode;
	if (max-1 < menu_bottom - menu_top) menu_bottom = menu_top + max-1;

	UINT32 num_modes = 0;
        for (UINT32 i = 0; i < ARRAY_SIZE(text_modes) && i < max; i++) {
	    // If mode is bad or rows/cols are invalid, go on
            if (cout->QueryMode(cout, i, &text_modes[num_modes].cols, &text_modes[num_modes].rows) != EFI_SUCCESS || 
	        ((text_modes[num_modes].cols < 10 || text_modes[num_modes].cols > 999) || 
	         (text_modes[num_modes].rows < 10 || text_modes[num_modes].rows > 999))) {
		continue;
	    }
	    text_modes[num_modes++].mode = i;
	}

	if (num_modes-1 < menu_bottom - menu_top) menu_bottom = menu_top + num_modes-1;
	UINTN menu_len = menu_bottom - menu_top + 1;	// 1-based offset

        // Highlight top menu row to start off
        cout->SetCursorPosition(cout, 0, menu_top);
        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
        printf_c16(u"Mode %d: %llux%llu", 
		   text_modes[0].mode, text_modes[0].cols, text_modes[0].rows);

        // Print other text mode infos
        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        for (UINT32 i = 1; i < menu_len; i++) 
            printf_c16(u"\r\nMode %d: %llux%llu", 
		       text_modes[i].mode, text_modes[i].cols, text_modes[i].rows);

        // Get input from user
        cout->SetCursorPosition(cout, 0, menu_top);
        bool getting_input = true;
        while (getting_input) {
            UINTN current_row = cout->Mode->CursorRow;

            EFI_INPUT_KEY key = get_key();
            switch (key.ScanCode) {
                case SCANCODE_ESC: return EFI_SUCCESS;  // ESC Key: Go back to main menu

                case SCANCODE_UP_ARROW:
                    if (current_row == menu_top && mode_index > 0) {
                        // Scroll menu up by decrementing all modes by 1
                        printf_c16(u"                    \r");  // Blank out mode text first

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        mode_index--;
                        printf_c16(u"Mode %d: %dx%d", 
                                   text_modes[mode_index].mode, 
				   text_modes[mode_index].cols, text_modes[mode_index].rows);

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                        UINTN temp_mode = mode_index + 1;
                        for (UINT32 i = 0; i < menu_len; i++, temp_mode++) {
                            printf_c16(u"\r\n                    \r"  // Blank out mode text first
                                       u"Mode %d: %dx%d\r", 
                                       text_modes[temp_mode].mode, 
				       text_modes[temp_mode].cols, text_modes[temp_mode].rows);
                        }

                        // Reset cursor to top of menu
                        cout->SetCursorPosition(cout, 0, menu_top);

                    } else if (current_row-1 >= menu_top) {
                        // De-highlight current row, move up 1 row, highlight new row
                        printf_c16(u"                    \r"    // Blank out mode text first
                                   u"Mode %d: %dx%d\r", 
                                   text_modes[mode_index].mode, 
			           text_modes[mode_index].cols, text_modes[mode_index].rows);

                        mode_index--;
                        current_row--;
                        cout->SetCursorPosition(cout, 0, current_row);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        printf_c16(u"                    \r"    // Blank out mode text first
                                   u"Mode %d: %dx%d\r", 
				   text_modes[mode_index].mode, 
				   text_modes[mode_index].cols, text_modes[mode_index].rows);
                    }

                    // Reset colors
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                    break;

                case SCANCODE_DOWN_ARROW:
                    if (current_row == menu_bottom && mode_index < num_modes-1) {
                        // Not at bottom of modes yet, scroll menu down by incrementing all modes by 1
                        mode_index -= menu_len - 1;

                        // Print modes up until the last menu row
                        cout->SetCursorPosition(cout, 0, menu_top);
                        for (UINT32 i = 0; i < menu_len; i++, mode_index++) {
                            printf_c16(u"                    \r"    // Blank out mode text first
                                       u"Mode %d: %dx%d\r\n", 
				       text_modes[mode_index].mode, 
				       text_modes[mode_index].cols, text_modes[mode_index].rows);
                        }

                        // Highlight last row
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        printf_c16(u"                    \r"    // Blank out mode text first
                                   u"Mode %d: %dx%d\r", 
				   text_modes[mode_index].mode, 
				   text_modes[mode_index].cols, text_modes[mode_index].rows);

                    } else if (current_row+1 <= menu_bottom) {
                        // De-highlight current row, move down 1 row, highlight new row
                        printf_c16(u"                    \r"    // Blank out mode text first
                                   u"Mode %d: %dx%d\r\n", 
				   text_modes[mode_index].mode, 
				   text_modes[mode_index].cols, text_modes[mode_index].rows);

                        mode_index++;
                        current_row++;
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        printf_c16(u"                    \r"    // Blank out mode text first
                                   u"Mode %d: %dx%d\r", 
				   text_modes[mode_index].mode, 
				   text_modes[mode_index].cols, text_modes[mode_index].rows);
                    }

                    // Reset colors
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                    break;

                default:
                    if (key.UnicodeChar == u'\r' && text_modes[mode_index].cols != 0) {	// Qemu can have invalid text modes
                        // Enter key, set Text mode
                        cout->SetMode(cout, text_modes[mode_index].mode);
                        cout->QueryMode(cout, text_modes[mode_index].mode, 
					&text_modes[mode_index].cols, &text_modes[mode_index].rows);

                        // Set global rows/cols values
                        text_rows = text_modes[mode_index].rows;
                        text_cols = text_modes[mode_index].cols;

			cout->ClearScreen(cout);

                        getting_input = false;  // Will leave input loop and redraw screen
                        mode_index = 0;         // Reset last selected mode in menu
                    }
                    break;
            }
        }
    }

    return EFI_SUCCESS;
}

// ====================
// Set Graphics Mode
// ====================
EFI_STATUS set_graphics_mode(void) {
    // Get GOP protocol via LocateProtocol()
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID; 
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode_info = NULL;
    UINTN mode_info_size = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    EFI_STATUS status = 0;
    UINTN mode_index = 0;   // Current mode within entire menu of GOP mode choices;

    // Store found GOP mode info
    typedef struct {
        UINT32 width;
        UINT32 height;
    } Gop_Mode_Info;

    Gop_Mode_Info gop_modes[50];

    status = bs->LocateProtocol(&gop_guid, NULL, (VOID **)&gop);
    if (EFI_ERROR(status)) {
        error(status, u"Could not locate GOP! :(\r\n");
        return status;
    }

    // Overall screen loop
    while (true) {
        cout->ClearScreen(cout);

        // Get current GOP mode information
        printf_c16(u"Graphics mode information:\r\n");
        status = gop->QueryMode(gop, 
                                gop->Mode->Mode, 
                                &mode_info_size, 
                                &mode_info);

        if (EFI_ERROR(status)) {
            error(status, u"Could not Query GOP Mode %u\r\n", gop->Mode->Mode);
            return status;
        }

        printf_c16(u"Max Mode: %d\r\n"
               u"Current Mode: %d\r\n"
               u"WidthxHeight: %ux%u\r\n"
               u"Framebuffer address: %x\r\n"
               u"Framebuffer size: %x\r\n"
               u"PixelFormat: %d\r\n"
               u"PixelsPerScanLine: %u\r\n",
               gop->Mode->MaxMode,
               gop->Mode->Mode,
               mode_info->HorizontalResolution, mode_info->VerticalResolution,
               gop->Mode->FrameBufferBase,
               gop->Mode->FrameBufferSize,
               mode_info->PixelFormat,
               mode_info->PixelsPerScanLine);

        cout->OutputString(cout, u"\r\nAvailable GOP modes:\r\n");

        // Get current text mode ColsxRows values
        UINTN menu_top = cout->Mode->CursorRow, menu_bottom = 0, max_cols;
        cout->QueryMode(cout, cout->Mode->Mode, &max_cols, &menu_bottom);

        // Print keybinds at bottom of screen
        cout->SetCursorPosition(cout, 0, menu_bottom-3);
        printf_c16(u"Up/Down Arrow = Move Cursor\r\n"
               u"Enter = Select\r\n"
               u"Escape = Go Back");

        cout->SetCursorPosition(cout, 0, menu_top);
        menu_bottom -= 5;   // Bottom of menu will be 2 rows above keybinds
        UINTN menu_len = menu_bottom - menu_top;

        // Get all available GOP modes' info
        const UINT32 max = gop->Mode->MaxMode;
        if (max < menu_len) {
            // Bound menu by actual # of available modes
            menu_bottom = menu_top + max-1;
            menu_len = menu_bottom - menu_top;  // Limit # of modes in menu to max mode - 1
        }

        for (UINT32 i = 0; i < ARRAY_SIZE(gop_modes) && i < max; i++) {
            gop->QueryMode(gop, i, &mode_info_size, &mode_info);

            gop_modes[i].width = mode_info->HorizontalResolution;
            gop_modes[i].height = mode_info->VerticalResolution;
        }

        // Highlight top menu row to start off
        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
        printf_c16(u"Mode %d: %dx%d", 0, gop_modes[0].width, gop_modes[0].height);

        // Print other text mode infos
        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        for (UINT32 i = 1; i < menu_len + 1; i++) 
            printf_c16(u"\r\nMode %d: %dx%d", i, gop_modes[i].width, gop_modes[i].height);

        // Get input from user 
        cout->SetCursorPosition(cout, 0, menu_top);
        bool getting_input = true;
        while (getting_input) {
            UINTN current_row = cout->Mode->CursorRow;

            EFI_INPUT_KEY key = get_key();
            switch (key.ScanCode) {
                case SCANCODE_ESC: return EFI_SUCCESS;  // ESC Key: Go back to main menu

                case SCANCODE_UP_ARROW:
                    if (current_row == menu_top && mode_index > 0) {
                        // Scroll menu up by decrementing all modes by 1
                        printf_c16(u"                    \r");  // Blank out mode text first

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        mode_index--;
                        printf_c16(u"Mode %d: %dx%d", 
                               mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                        UINTN temp_mode = mode_index + 1;
                        for (UINT32 i = 0; i < menu_len; i++, temp_mode++) {
                            printf_c16(u"\r\n                    \r"  // Blank out mode text first
                                   u"Mode %d: %dx%d\r", 
                                   temp_mode, gop_modes[temp_mode].width, gop_modes[temp_mode].height);
                        }

                        // Reset cursor to top of menu
                        cout->SetCursorPosition(cout, 0, menu_top);

                    } else if (current_row-1 >= menu_top) {
                        // De-highlight current row, move up 1 row, highlight new row
                        printf_c16(u"                    \r"    // Blank out mode text first
                               u"Mode %d: %dx%d\r", 
                               mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);

                        mode_index--;
                        current_row--;
                        cout->SetCursorPosition(cout, 0, current_row);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        printf_c16(u"                    \r"    // Blank out mode text first
                               u"Mode %d: %dx%d\r", 
                               mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);
                    }

                    // Reset colors
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                    break;

                case SCANCODE_DOWN_ARROW:
                    // NOTE: Max valid GOP mode is ModeMax-1 per UEFI spec
                    if (current_row == menu_bottom && mode_index < max-1) {
                        // Scroll menu down by incrementing all modes by 1
                        mode_index -= menu_len - 1;

                        // Reset cursor to top of menu
                        cout->SetCursorPosition(cout, 0, menu_top);

                        // Print modes up until the last menu row
                        for (UINT32 i = 0; i < menu_len; i++, mode_index++) {
                            printf_c16(u"                    \r"    // Blank out mode text first
                                   u"Mode %d: %dx%d\r\n", 
                                   mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);
                        }

                        // Highlight last row
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        printf_c16(u"                    \r"    // Blank out mode text first
                               u"Mode %d: %dx%d\r", 
                               mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);

                    } else if (current_row+1 <= menu_bottom) {
                        // De-highlight current row, move down 1 row, highlight new row
                        printf_c16(u"                    \r"    // Blank out mode text first
                               u"Mode %d: %dx%d\r\n", 
                               mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);

                        mode_index++;
                        current_row++;
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
                        printf_c16(u"                    \r"    // Blank out mode text first
                               u"Mode %d: %dx%d\r", 
                               mode_index, gop_modes[mode_index].width, gop_modes[mode_index].height);
                    }

                    // Reset colors
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                    break;

                default:
                    if (key.UnicodeChar == u'\r') {
                        // Enter key, set GOP mode
                        gop->SetMode(gop, mode_index);
                        gop->QueryMode(gop, mode_index, &mode_info_size, &mode_info);

                        // Clear GOP screen 
                        EFI_GRAPHICS_OUTPUT_BLT_PIXEL px = px_BLUE;
                        gop->Blt(gop, &px, EfiBltVideoFill, 
                                 0, 0,  // Origin BLT BUFFER X,Y
                                 0, 0,  // Destination screen X,Y
                                 mode_info->HorizontalResolution, mode_info->VerticalResolution,
                                 0);

                        getting_input = false;  // Will leave input loop and redraw screen
                        mode_index = 0;         // Reset last selected mode in menu
                    }
                    break;
            }
        }
    }

    return EFI_SUCCESS;
}

// ===================================================================
// Test mouse, touchscreen & various cursor/pointer support using 
//   Simple Pointer Protocol (SPP) & Absolute Pointer Protocol (APP)
// ===================================================================
EFI_STATUS test_mouse(void) {
    EFI_GUID spp_guid = EFI_SIMPLE_POINTER_PROTOCOL_GUID;
    EFI_SIMPLE_POINTER_PROTOCOL *spp[5];

    UINTN spp_handles = 0, app_handles = 0;
    EFI_HANDLE *spp_handle_buf = NULL, *app_handle_buf = NULL;

    EFI_STATUS status = 0;

    INTN cursor_size = 8;
    INTN cursor_x = 0, cursor_y = 0;

    EFI_GUID app_guid = EFI_ABSOLUTE_POINTER_PROTOCOL_GUID;
    EFI_ABSOLUTE_POINTER_PROTOCOL *app[5];

    typedef enum {
        CIN = 0,
        SPP = 1,
        APP = 2,
    } INPUT_TYPE;

    typedef struct {
        EFI_EVENT wait_event;
        INPUT_TYPE type;

        union {
            EFI_SIMPLE_POINTER_PROTOCOL   *spp;
            EFI_ABSOLUTE_POINTER_PROTOCOL *app;
        };
    } INPUT_PROTOCOL;

    INPUT_PROTOCOL input_protocols[11];
    UINTN num_protocols = 0;


    input_protocols[num_protocols++] = (INPUT_PROTOCOL){
        .wait_event = cin->WaitForKey,
        .type = CIN,
        .spp = NULL,
    };


    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode_info = NULL;

    UINTN mode_info_size = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    UINTN mode_index = 0;


    status = bs->LocateProtocol(&gop_guid, NULL, (VOID **)&gop);

    if (EFI_ERROR(status)) {
        printf_c16(
            u"ERROR (%llu): Could not locate GOP! :(\r\n",
            status
        );

        return status;
    }


    if (gop->Mode != NULL) {
        mode_index = gop->Mode->Mode;
    }


    gop->QueryMode(
        gop,
        mode_index,
        &mode_info_size,
        &mode_info
    );


    cout->ClearScreen(cout);

    BOOLEAN found_mode = FALSE;


    status = bs->LocateHandleBuffer(
        ByProtocol,
        &spp_guid,
        NULL,
        &spp_handles,
        &spp_handle_buf
    );


    if (EFI_ERROR(status)) {
        printf_c16(
            u"ERROR (%llu): Could not locate Simple Pointer Protocol handle buffer.\r\n",
            status
        );

        goto get_app;
    }


    for (UINTN i = 0; i < spp_handles; i++) {

        status = bs->OpenProtocol(
            spp_handle_buf[i],
            &spp_guid,
            (VOID **)&spp[i],
            image,
            NULL,
            EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
        );


        if (EFI_ERROR(status)) {

            printf_c16(
                u"ERROR (%llu): Could not Open Simple Pointer Protocol on handle.\r\n",
                status
            );

            continue;
        }


        spp[i]->Reset(spp[i], TRUE);


        printf_c16(
            u"SPP %u; Resolution X: %u, Y: %u, Z: %u, LButton: %u, RButton: %u\r\n",
            i,
            spp[i]->Mode->ResolutionX,
            spp[i]->Mode->ResolutionY,
            spp[i]->Mode->ResolutionZ,
            spp[i]->Mode->LeftButton,
            spp[i]->Mode->RightButton
        );


        if (spp[i]->Mode->ResolutionX <= 65536)
        {
            found_mode = TRUE;

            input_protocols[num_protocols++] = (INPUT_PROTOCOL){
                .wait_event = spp[i]->WaitForInput,
                .type = SPP,
                .spp = spp[i]
            };
        }
    }


    if (!found_mode) {
        printf_c16(
            u"\r\nCould not find any valid SPP Mode.\r\n"
        );
    }
    get_app:

    found_mode = FALSE;


    status = bs->LocateHandleBuffer(
        ByProtocol,
        &app_guid,
        NULL,
        &app_handles,
        &app_handle_buf
    );


    if (EFI_ERROR(status)) {

        printf_c16(
            u"ERROR (%llu): Could not locate Absolute Pointer Protocol handle buffer.\r\n",
            status
        );

        goto after_app;
    }


    printf_c16(u"\r\n");


    for (UINTN i = 0; i < app_handles; i++) {

        status = bs->OpenProtocol(
            app_handle_buf[i],
            &app_guid,
            (VOID **)&app[i],
            image,
            NULL,
            EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
        );


        if (EFI_ERROR(status)) {

            printf_c16(
                u"ERROR (%llu): Could not Open Absolute Pointer Protocol on handle.\r\n",
                status
            );

            continue;
        }


        app[i]->Reset(app[i], TRUE);


        printf_c16(
            u"APP %u; Min X: %u, Y: %u, Z: %u, Max X: %u, Y: %u, Z: %u, Attributes: %b\r\n",
            i,
            app[i]->Mode->AbsoluteMinX,
            app[i]->Mode->AbsoluteMinY,
            app[i]->Mode->AbsoluteMinZ,
            app[i]->Mode->AbsoluteMaxX,
            app[i]->Mode->AbsoluteMaxY,
            app[i]->Mode->AbsoluteMaxZ,
            app[i]->Mode->Attributes
        );


        if (app[i]->Mode->AbsoluteMaxX < 65536) {

            found_mode = TRUE;


            input_protocols[num_protocols++] = (INPUT_PROTOCOL){
                .wait_event = app[i]->WaitForInput,
                .type = APP,
                .app = app[i]
            };
        }
    }


    if (!found_mode) {

        printf_c16(
            u"Could not find any valid APP Mode.\r\n"
        );
    }


after_app:

    if (num_protocols == 0) {

        printf_c16(
            u"Could not find any Simple or Absolute Pointer Protocols.\r\n"
        );

        goto done;
    }


    INT32 xres = mode_info->HorizontalResolution;
    INT32 yres = mode_info->VerticalResolution;


    cursor_x = (xres / 2) - (cursor_size / 2);
    cursor_y = (yres / 2) - (cursor_size / 2);


    printf_c16(
        u"\r\nMouse Xpos: %d, Ypos: %d, Xmm: %d, Ymm: %d, LB: %u, RB: %u\r",
        cursor_x,
        cursor_y,
        0,
        0,
        0,
        0
    );


    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *fb =
        (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gop->Mode->FrameBufferBase;


    for (INTN y = 0; y < cursor_size; y++) {

        for (INTN x = 0; x < cursor_size; x++) {

            save_buffer[(y * cursor_size) + x] =
                fb[(mode_info->PixelsPerScanLine *
                    (cursor_y + y)) +
                    (cursor_x + x)];


            EFI_GRAPHICS_OUTPUT_BLT_PIXEL csr_px =
                cursor_buffer[(y * cursor_size) + x];


            fb[(mode_info->PixelsPerScanLine *
                (cursor_y + y)) +
                (cursor_x + x)] = csr_px;
        }
    }


    EFI_EVENT events[11];


    for (UINTN i = 0; i < num_protocols; i++)
        events[i] = input_protocols[i].wait_event;


    while (TRUE) {

        UINTN index = 0;


        bs->WaitForEvent(
            num_protocols,
            events,
            &index
        );


        if (input_protocols[index].type == CIN) {

            EFI_INPUT_KEY key;


            cin->ReadKeyStroke(
                cin,
                &key
            );


            if (key.ScanCode == SCANCODE_ESC)
                break;
        }
                else if (input_protocols[index].type == SPP) {

            EFI_SIMPLE_POINTER_STATE state;

            EFI_SIMPLE_POINTER_PROTOCOL *active_spp =
                input_protocols[index].spp;


            active_spp->GetState(
                active_spp,
                &state
            );


            double xmm_float =
                (double)state.RelativeMovementX /
                (double)active_spp->Mode->ResolutionX;


            double ymm_float =
                (double)state.RelativeMovementY /
                (double)active_spp->Mode->ResolutionY;


            printf_c16(
                u"                                                                      \r"
            );


            printf_c16(
                u"Mouse Xpos: %d, Ypos: %d, Xmm: %.4f, Ymm: %.4f, LB: %b, RB: %b\r",
                cursor_x,
                cursor_y,
                xmm_float,
                ymm_float,
                state.LeftButton,
                state.RightButton
            );


            const double xres_mm_px =
                mode_info->HorizontalResolution * 0.02;


            const double yres_mm_px =
                mode_info->VerticalResolution * 0.02;


            for (INTN y = 0; y < cursor_size; y++) {

                for (INTN x = 0; x < cursor_size; x++) {

                    fb[(mode_info->PixelsPerScanLine *
                        (cursor_y + y)) +
                        (cursor_x + x)] =
                        save_buffer[(y * cursor_size) + x];
                }
            }


            cursor_x += (INTN)(xres_mm_px * xmm_float);
            cursor_y += (INTN)(yres_mm_px * ymm_float);


            if (cursor_x < 0)
                cursor_x = 0;

            if (cursor_x > xres - cursor_size)
                cursor_x = xres - cursor_size;

            if (cursor_y < 0)
                cursor_y = 0;

            if (cursor_y > yres - cursor_size)
                cursor_y = yres - cursor_size;


            for (INTN y = 0; y < cursor_size; y++) {

                for (INTN x = 0; x < cursor_size; x++) {

                    save_buffer[(y * cursor_size) + x] =
                        fb[(mode_info->PixelsPerScanLine *
                            (cursor_y + y)) +
                            (cursor_x + x)];


                    EFI_GRAPHICS_OUTPUT_BLT_PIXEL csr_px =
                        cursor_buffer[(y * cursor_size) + x];


                    fb[(mode_info->PixelsPerScanLine *
                        (cursor_y + y)) +
                        (cursor_x + x)] = csr_px;
                }
            }
        }


        else if (input_protocols[index].type == APP) {

            EFI_ABSOLUTE_POINTER_STATE state;

            EFI_ABSOLUTE_POINTER_PROTOCOL *active_app =
                input_protocols[index].app;


            active_app->GetState(
                active_app,
                &state
            );


            printf_c16(
                u"                                                                      \r"
            );


            printf_c16(
                u"Ptr Xpos: %u, Ypos: %u, Zpos: %u, Buttons: %b\r",
                state.CurrentX,
                state.CurrentY,
                state.CurrentZ,
                state.ActiveButtons
            );


            for (INTN y = 0; y < cursor_size; y++) {

                for (INTN x = 0; x < cursor_size; x++) {

                    fb[(mode_info->PixelsPerScanLine *
                        (cursor_y + y)) +
                        (cursor_x + x)] =
                        save_buffer[(y * cursor_size) + x];
                }
            }


            float x_app_ratio =
                (float)mode_info->HorizontalResolution /
                (float)active_app->Mode->AbsoluteMaxX;


            float y_app_ratio =
                (float)mode_info->VerticalResolution /
                (float)active_app->Mode->AbsoluteMaxY;


            cursor_x =
                (INTN)((float)state.CurrentX * x_app_ratio);


            cursor_y =
                (INTN)((float)state.CurrentY * y_app_ratio);


            if (cursor_x < 0)
                cursor_x = 0;

            if (cursor_x > xres - cursor_size)
                cursor_x = xres - cursor_size;

            if (cursor_y < 0)
                cursor_y = 0;

            if (cursor_y > yres - cursor_size)
                cursor_y = yres - cursor_size;


            for (INTN y = 0; y < cursor_size; y++) {

                for (INTN x = 0; x < cursor_size; x++) {

                    save_buffer[(y * cursor_size) + x] =
                        fb[(mode_info->PixelsPerScanLine *
                            (cursor_y + y)) +
                            (cursor_x + x)];


                    EFI_GRAPHICS_OUTPUT_BLT_PIXEL csr_px =
                        cursor_buffer[(y * cursor_size) + x];


                    fb[(mode_info->PixelsPerScanLine *
                        (cursor_y + y)) +
                        (cursor_x + x)] = csr_px;
                }
            }
        }
    }


done:

    if (spp_handle_buf) {

        for (UINTN i = 0; i < spp_handles; i++) {

            bs->CloseProtocol(
                spp_handle_buf[i],
                &spp_guid,
                image,
                NULL
            );
        }

        bs->FreePool(spp_handle_buf);
    }


    if (app_handle_buf) {

        for (UINTN i = 0; i < app_handles; i++) {

            bs->CloseProtocol(
                app_handle_buf[i],
                &app_guid,
                image,
                NULL
            );
        }

        bs->FreePool(app_handle_buf);
    }


    return EFI_SUCCESS;
}

// =====================================================
// Test if EFI_SIMPLE_NETWORK_PROTOCOL is found or not
// =====================================================
EFI_STATUS test_network(void) {
    cout->ClearScreen(cout);

    EFI_GUID netGuid = EFI_SIMPLE_NETWORK_PROTOCOL_GUID;
    EFI_SIMPLE_NETWORK_PROTOCOL* netProtocol;
    EFI_STATUS status = bs->LocateProtocol(&netGuid, NULL, (VOID**)&netProtocol);
    if(EFI_ERROR(status)) {
        printf_c16(u"ERROR: Network protocol(s) not found.\r\n");
        printf(L"Press enter to continue...");
    }
    else {
        printf_c16(u"Network protocol(s) found.\r\n");
        printf(L"Press enter to continue...");
    }
    get_key();
    return status;
}

// ===========================================================
// Timer function to print current date/time every 1 second
// ===========================================================
VOID print_datetime(void) {
    UINTN max_cols = 0, max_rows = 0;
    cout->QueryMode(cout, cout->Mode->Mode, &max_cols, &max_rows);

    UINTN save_col = cout->Mode->CursorColumn;
    UINTN save_row = cout->Mode->CursorRow;

    EFI_TIME time;
    EFI_TIME_CAPABILITIES caps;
    rs->GetTime(&time, &caps);

    cout->SetCursorPosition(cout, max_cols - 20, max_rows - 1);
    printf_c16(u"%u-%c%u-%c%u %c%u:%c%u:%c%u",
        time.Year,
        time.Month  < 10 ? u'0' : u'\0', time.Month,
        time.Day    < 10 ? u'0' : u'\0', time.Day,
        time.Hour   < 10 ? u'0' : u'\0', time.Hour,
        time.Minute < 10 ? u'0' : u'\0', time.Minute,
        time.Second < 10 ? u'0' : u'\0', time.Second);

    cout->SetCursorPosition(cout, save_col, save_row);
    get_key();
}

// ================================================
// Read & print files in the EFI System Partition
// ================================================
EFI_STATUS read_esp_files(void) {
    EFI_STATUS status = EFI_SUCCESS;

    // Get ESP root directory
    EFI_FILE_PROTOCOL *dirp = esp_root_dir();
    if (!dirp) {
        error(0, u"Could not get ESP root directory.\r\n");
        goto done;
    }

    // Start at root directory
    CHAR16 current_directory[256];
    strcpy_c16(current_directory, u"/");    

    // Print dir entries for currently opened directory
    // Overall input loop
    INT32 csr_row = 1;
    while (true) {
        cout->ClearScreen(cout);
        printf_c16(u"%s:\r\n", current_directory);

        INT32 num_entries = 0;
        EFI_FILE_INFO file_info;

        dirp->SetPosition(dirp, 0);                 // Reset to start of directory entries
        UINTN buf_size = sizeof file_info;
        dirp->Read(dirp, &buf_size, &file_info);
        while (buf_size > 0) {
            num_entries++;

            // Got next dir entry, print info
            if (csr_row == cout->Mode->CursorRow) {
                // Highlight row cursor/user is on
                cout->SetAttribute(cout, EFI_TEXT_ATTR(HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR));
            }

            printf_c16(u"%s %s\r\n", 
                   (file_info.Attribute & EFI_FILE_DIRECTORY) ? u"[DIR] " : u"[FILE]",
                   file_info.FileName);

            if (csr_row+1 == cout->Mode->CursorRow) {
                // De-highlight rows after cursor
                cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
            }

            buf_size = sizeof file_info;
            dirp->Read(dirp, &buf_size, &file_info);
        }

        EFI_INPUT_KEY key = get_key();
        switch (key.ScanCode) {
            case SCANCODE_ESC:
                // ESC Key, exit and go back to main menu
                goto done;
                break;

            case SCANCODE_UP_ARROW:
            case SCANCODE_DOWN_ARROW:
                // Go up or down 1 row in range [1:num_entries] (circular buffer)
                csr_row = (key.ScanCode == SCANCODE_UP_ARROW) 
                          ? ((csr_row-1 + num_entries-1) % num_entries) + 1
                          : (csr_row % num_entries) + 1;
                break;

            default:
                if (key.UnicodeChar == u'\r') {
                    // Enter key: 
                    //   for a directory, enter that directory and iterate the loop
                    //   for a file, print the file contents to screen

                    // Get directory entry under user cursor row
                    dirp->SetPosition(dirp, 0);  // Reset to start of directory entries
                    INT32 i = 0;  
                    do {
                        buf_size = sizeof file_info;
                        dirp->Read(dirp, &buf_size, &file_info);
                        i++;
                    } while (i < csr_row);

                    if (file_info.Attribute & EFI_FILE_DIRECTORY) {
                        // Directory, open and enter this new directory
                        EFI_FILE_PROTOCOL *new_dir;
                        status = dirp->Open(dirp, 
                                            &new_dir, 
                                            file_info.FileName, 
                                            EFI_FILE_MODE_READ,
                                            0);

                        if (EFI_ERROR(status)) {
                            error(status, u"Could not open new directory %s\r\n", file_info.FileName);
                            goto done;
                        }

                        dirp->Close(dirp);  // Close last opened dir
                        dirp = new_dir;     // Set new opened dir
                        csr_row = 1;        // Reset user row to first entry in new directory

                        // Set new path for current directory
                        if (!strncmp_u16(file_info.FileName, u".", 2)) {
                            // Current directory, do nothing

                        } else if (!strncmp_u16(file_info.FileName, u"..", 3)) {
                            // Parent directory, go back up and remove dir name from path
                            CHAR16 *pos = strrchr_u16(current_directory, u'/');
                            if (pos == current_directory) pos++;    // Move past initial root dir '/'

                            *pos = u'\0';

                        } else {
                            // Go into nested directory, add on to current string
                            if (current_directory[1] != u'\0') {
                                strcat_c16(current_directory, u"/"); 
                            }
                            strcat_c16(current_directory, file_info.FileName);
                        }
                        continue;   // Continue overall loop and print new directory entries
                    } 

                    // Else this is a file, print contents:
                    // Allocate buffer for file
                    VOID *buffer = NULL;
                    buf_size = file_info.FileSize;
                    status = bs->AllocatePool(EfiLoaderData, buf_size, &buffer);
                    if (EFI_ERROR(status)) {
                        error(status, u"Could not allocate memory for file %s\r\n", file_info.FileName);
                        goto done;
                    }

                    // Open file
                    EFI_FILE_PROTOCOL *file = NULL;
                    status = dirp->Open(dirp, 
                                        &file, 
                                        file_info.FileName, 
                                        EFI_FILE_MODE_READ,
                                        0);

                    if (EFI_ERROR(status)) {
                        error(status, u"Could not open file %s\r\n", file_info.FileName);
                        goto done;
                    }

                    // Read file into buffer
                    status = dirp->Read(file, &buf_size, buffer);
                    if (EFI_ERROR(status)) {
                        error(status, u"Could not read file %s into buffer.\r\n", file_info.FileName);
                        goto done;
                    } 

                    if (buf_size != file_info.FileSize) {
                        error(0, u"Could not read all of file %s into buffer.\r\n" 
                              u"Bytes read: %u, Expected: %u\r\n",
                              file_info.FileName, buf_size, file_info.FileSize);
                        goto done;
                    }

                    // Print buffer contents
                    printf_c16(u"\r\nFile Contents:\r\n");

                    char *pos = (char *)buffer;
                    for (UINTN bytes = buf_size; bytes > 0; bytes--) {
                        CHAR16 str[2];
                        str[0] = *pos;
                        str[1] = u'\0';
                        if (*pos == '\n') {
                            // Convert LF newline to CRLF
                            printf_c16(u"\r\n");
                        } else {
                            printf_c16(u"%s", str);
                        }

                        pos++;
                    }

                    printf_c16(u"\r\n\r\nPress any key to continue...\r\n");
                    get_key();

                    // Free memory for file when done
                    bs->FreePool(buffer);

                    // Close file handle
                    dirp->Close(file);
                }
                break;
        }
    }

    done:
    if (dirp) dirp->Close(dirp);    // Cleanup directory pointer
    return status;
}

// ======================================================================
// Print Block IO Partitions using Block IO and Parition Info Protocols
// ======================================================================
EFI_STATUS print_block_io_partitions(void) {
    EFI_STATUS status = EFI_SUCCESS;

    cout->ClearScreen(cout);

    EFI_GUID bio_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
    EFI_BLOCK_IO_PROTOCOL *biop;
    UINTN num_handles = 0;
    EFI_HANDLE *handle_buffer = NULL;

    // Get media ID for this disk image first, to compare to others in output
    UINT32 this_image_media_id = 0;
    status = get_disk_image_mediaID(&this_image_media_id);
    if (EFI_ERROR(status)) {
        error(status, u"Could not get disk image media ID.\r\n");
        return status;
    }

    // Loop through and print all partition information found
    status = bs->LocateHandleBuffer(ByProtocol, &bio_guid, NULL, &num_handles, &handle_buffer);
    if (EFI_ERROR(status)) {
        error(status, u"Could not locate any Block IO Protocols.\r\n");
        return status;
    }

    UINT32 last_media_id = -1;  // Keep track of currently opened Media info
    for (UINTN i = 0; i < num_handles; i++) {
        status = bs->OpenProtocol(handle_buffer[i], 
                                  &bio_guid,
                                  (VOID **)&biop,
                                  image,
                                  NULL,
                                  EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if (EFI_ERROR(status)) {
            error(status, u"Could not Open Block IO protocol on handle %u.\r\n", i);
            continue;
        }

        // Print Block IO Media Info for this Disk/partition
        if (last_media_id != biop->Media->MediaId) {
            last_media_id = biop->Media->MediaId;   
            printf_c16(u"Media ID: %u %s\r\n", 
                   last_media_id, 
                   (last_media_id == this_image_media_id ? u"(Disk Image)" : u""));
        }

        if (biop->Media->LastBlock == 0) {
            // Only really care about partitions/disks above 1 block in size
            continue;
        }

        printf_c16(u"Rmv: %s, Pr: %s, LglPrt: %s, RdOnly: %s, Wrt$: %s\r\n"
               u"BlkSz: %u, IoAln: %u, LstBlk: %u, LwLBA: %u, LglBlkPerPhys: %u\r\n"
               u"OptTrnLenGran: %u\r\n",
               biop->Media->RemovableMedia   ? u"Y" : u"N",
               biop->Media->MediaPresent     ? u"Y" : u"N",
               biop->Media->LogicalPartition ? u"Y" : u"N",
               biop->Media->ReadOnly         ? u"Y" : u"N",
               biop->Media->WriteCaching     ? u"Y" : u"N",

               biop->Media->BlockSize,
               biop->Media->IoAlign,
               biop->Media->LastBlock,
               biop->Media->LowestAlignedLba,                   
               biop->Media->LogicalBlocksPerPhysicalBlock,     
               biop->Media->OptimalTransferLengthGranularity);

        // Print type of partition e.g. ESP or Data or Other
        if (!biop->Media->LogicalPartition) printf_c16(u"<Entire Disk>\r\n");
        else {
            // Get partition info protocol for this partition
            EFI_GUID pi_guid = EFI_PARTITION_INFO_PROTOCOL_GUID;
            EFI_PARTITION_INFO_PROTOCOL *pip = NULL;
            status = bs->OpenProtocol(handle_buffer[i], 
                                      &pi_guid,
                                      (VOID **)&pip,
                                      image,
                                      NULL,
                                      EFI_OPEN_PROTOCOL_GET_PROTOCOL);

            if (EFI_ERROR(status)) {
                error(status, u"Could not Open Partition Info protocol on handle %u.\r\n", i);
            } else {
                if      (pip->Type == PARTITION_TYPE_OTHER) printf_c16(u"<Other Type>\r\n");
                else if (pip->Type == PARTITION_TYPE_MBR)   printf_c16(u"<MBR>\r\n");
                else if (pip->Type == PARTITION_TYPE_GPT) {
                    if (pip->System == 1) printf_c16(u"<EFI System Partition>\r\n");
                    else {
                        // Compare Gpt.PartitionTypeGUID with known values
                        EFI_GUID data_guid = BASIC_DATA_GUID;
                        if (!memcmp(&pip->Info.Gpt.PartitionTypeGUID, &data_guid, sizeof(EFI_GUID))) 
                            printf_c16(u"<Basic Data>\r\n");
                        else
                            printf_c16(u"<Other GPT Type>\r\n");
                    }
                }
            }
        }

        printf_c16(u"\r\n");    // Separate each block of text visually 
    }

    printf_c16(u"Press any key to go back..\r\n");
    get_key();
    return EFI_SUCCESS;
}


// ====================
// Print Memory Map
// ====================
EFI_STATUS print_memory_map(void) { 
    cout->ClearScreen(cout);

    Memory_Map_Info mmap = {0};
    get_memory_map(&mmap);

    // Print memory map descriptor values
    printf_c16(u"Memory map: Size %u, Descriptor size: %u, # of descriptors: %u, key: %x\r\n",
            mmap.size, mmap.desc_size, mmap.size / mmap.desc_size, mmap.key);

    UINTN usable_bytes = 0; // "Usable" memory for an OS or similar, not firmware/device reserved
    for (UINTN i = 0; i < mmap.size / mmap.desc_size; i++) {
        EFI_MEMORY_DESCRIPTOR *desc = 
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mmap.map + (i * mmap.desc_size));

        printf_c16(u"%u: Typ: %u, Phy: %x, Vrt: %x, Pgs: %u, Att: %x\r\n",
                i,
                desc->Type, 
                desc->PhysicalStart, 
                desc->VirtualStart, 
                desc->NumberOfPages, 
                desc->Attribute);

        // Add to usable memory count depending on type
        if (desc->Type == EfiLoaderCode         || 
            desc->Type == EfiLoaderData         || 
            desc->Type == EfiBootServicesCode   || 
            desc->Type == EfiBootServicesData   || 
            desc->Type == EfiConventionalMemory || 
            desc->Type == EfiPersistentMemory) {

            usable_bytes += desc->NumberOfPages * 4096;
        }

        // Pause if reached bottom of screen
        if (cout->Mode->CursorRow >= text_rows-2) {
            printf_c16(u"Press any key to continue...\r\n");
            get_key();
            cout->ClearScreen(cout);
        }
    }

    printf_c16(u"\r\nUsable memory: %u / %u MiB / %u GiB\r\n",
            usable_bytes, usable_bytes / (1024 * 1024), usable_bytes / (1024 * 1024 * 1024));

    // Free allocated buffer for memory map
    bs->FreePool(mmap.map);

    printf_c16(u"\r\nPress any key to go back...\r\n");
    get_key();
    return EFI_SUCCESS;
}

// =======================================
// Print configuration table GUID values
// =======================================
EFI_STATUS print_config_tables(void) { 
    cout->ClearScreen(cout);

    printf_c16(u"Configuration Table GUIDs:\r\n");
    for (UINTN i = 0; i < st->NumberOfTableEntries; i++) {
        EFI_GUID guid = st->ConfigurationTable[i].VendorGuid;
        print_guid(guid);

        // Print GUID name if available, else unknown
        UINTN j = 0;
        bool found = false;
        for (; j < ARRAY_SIZE(config_table_guids_and_strings); j++) {
            if (!memcmp(&guid, &config_table_guids_and_strings[j].guid, sizeof guid)) {
                found = true;
                break;
            }
        }
        printf_c16(u"(%s)\r\n\r\n", 
               found ? config_table_guids_and_strings[j].string : u"Unknown GUID Value");

        // Pause at bottom of screen
        if (cout->Mode->CursorRow >= text_rows-2) {
            printf_c16(u"Press any key to continue...\r\n");
            get_key();
            cout->ClearScreen(cout);
        }
    }

    printf_c16(u"\r\nPress any key to go back...\r\n");
    get_key();
    return EFI_SUCCESS;
}

// =======================================
// Print configuration table GUID values
// =======================================
EFI_STATUS print_acpi_tables(void) { 
    cout->ClearScreen(cout);

    // Check for ACPI 2.0+ table
    EFI_GUID acpi_guid = EFI_ACPI_TABLE_GUID;
    VOID *rsdp_ptr = get_config_table_by_guid(acpi_guid);
    bool acpi_20 = false;
    if (rsdp_ptr) {
        printf_c16(u"ACPI 2.0 Table found at %#x\r\n", rsdp_ptr);
        acpi_20 = true;
    } else {
        // Check for ACPI 1.0 table as fallback
        acpi_guid = (EFI_GUID)ACPI_TABLE_GUID;
        rsdp_ptr = get_config_table_by_guid(acpi_guid);
        if (rsdp_ptr) {
            printf_c16(u"ACPI 1.0 Table found at %#x\r\n", rsdp_ptr);
        } else {
            error(0, u"Could not find ACPI configuration table\r\n");
            return 1;
        }
    }

    // Print RSDP
    UINT8 *rsdp = rsdp_ptr;
    printf_c16(u"RSDP:\r\n"
           u"Signature: %.8hhs\r\n"
           u"Checksum: %hhu\r\n"
           u"OEMID: %.6hhs\r\n"
           u"Revision: %hhu\r\n"
           u"RSDT Address: %x\r\n",
           &rsdp[0], 
           rsdp[8],
           &rsdp[9],
           rsdp[15],
           *(UINT32 *)&rsdp[16]);

    if (acpi_20) {
        printf_c16(u"Length: %u\r\n"
               u"XSDT Address: %x\r\n"
               u"Extended Checksum: %hhu\r\n",
               *(UINT32 *)&rsdp[20],
               *(UINT64 *)&rsdp[24],
               rsdp[32]);
    } 

    printf_c16(u"\r\nPress any key to print RSDT/XSDT...\r\n");
    get_key();

    // Uncomment this line to use RSDT instead of XSDT
    //acpi_20 = false;
    
    ACPI_TABLE_HEADER *header = NULL;
    if (acpi_20) {
        // Print XSDT header
        UINT64 xsdt_address = *(UINT64 *)&rsdp[24];
        header = (ACPI_TABLE_HEADER *)xsdt_address;
        print_acpi_table_header(*header);

        // Print XSDT entry signatures
        printf_c16(u"\r\nPress any key to print entries...\r\n");
        get_key();

        cout->ClearScreen(cout);
        printf_c16(u"Entries:\r\n");
        UINT64 *entry = (UINT64 *)((UINT8 *)header + sizeof *header); 
        for (UINTN i = 0; i < (header->length - sizeof *header) / 8; i++) {
            ACPI_TABLE_HEADER table_header = *(ACPI_TABLE_HEADER *)entry[i];
            printf_c16(u"%.4hhs\r\n", &table_header.signature[0]);

            if (cout->Mode->CursorRow >= text_rows-2) {
                printf_c16(u"Press any key to continue...\r\n");
                get_key();
                cout->ClearScreen(cout);
            }
        }

        printf_c16(u"\r\nPress any key to print next table...\r\n");
        get_key();

        // Loop and print each ACPI table
        for (UINTN i = 0; i < (header->length - sizeof *header) / 8; i++) {
            cout->ClearScreen(cout);

            // Print header
            ACPI_TABLE_HEADER table_header = *(ACPI_TABLE_HEADER *)entry[i];
            print_acpi_table_header(table_header);

            // TODO: Print specific table info ?

            printf_c16(u"\r\nPress any key to print next table...\r\n");
            get_key();
        }

    } else {
        // Print RSDT header
        UINT32 rsdt_address = *(UINT32 *)&rsdp[16];

        // The extra (UINTN) casts are to avoid compiler warnings about casting smaller 
        //   int types to pointer
        header = (ACPI_TABLE_HEADER *)(UINTN)rsdt_address;
        print_acpi_table_header(*header);

        // Print RSDT entry signatures
        printf_c16(u"\r\nPress any key to print entries...\r\n");
        get_key();

        cout->ClearScreen(cout);
        printf_c16(u"Entries:\r\n");
        UINT32 *entry = (UINT32 *)((UINT8 *)header + sizeof *header); 
        for (UINTN i = 0; i < (header->length - sizeof *header) / 4; i++) {
            ACPI_TABLE_HEADER table_header = *(ACPI_TABLE_HEADER *)(UINTN)entry[i];
            printf_c16(u"%.4hhs\r\n", &table_header.signature[0]);

            if (cout->Mode->CursorRow >= text_rows-2) {
                printf_c16(u"Press any key to continue...\r\n");
                get_key();
                cout->ClearScreen(cout);
            }
        }

        printf_c16(u"\r\nPress any key to print next table...\r\n");
        get_key();

        // Loop and print each ACPI table
        for (UINTN i = 0; i < (header->length - sizeof *header) / 4; i++) {
            cout->ClearScreen(cout);

            // Print header
            ACPI_TABLE_HEADER table_header = *(ACPI_TABLE_HEADER *)(UINTN)entry[i];
            print_acpi_table_header(table_header);

            // TODO: Print specific table info ?

            printf_c16(u"\r\nPress any key to print next table...\r\n");
            get_key();
        }
    }

    printf_c16(u"\r\nPress any key to go back...\r\n");
    get_key();
    return EFI_SUCCESS;
}

// ================================
// Print all EFI Global Variables
// ================================
EFI_STATUS print_efi_global_variables(void) { 
    cout->ClearScreen(cout);

    UINTN var_name_size = 0;
    CHAR16 *var_name_buf = 0;
    EFI_GUID vendor_guid = {0};
    EFI_STATUS status = EFI_SUCCESS;

    var_name_size = 2;
    status = bs->AllocatePool(EfiLoaderData, var_name_size, (VOID **)&var_name_buf);
    if (EFI_ERROR(status)) {
        error(status, u"Could not allocate 2 bytes?!\r\n");
        return status;
    }

    // Set variable name to point to initial single null byte, to start off call to get list of
    //   variable names
    *var_name_buf = u'\0';

    status = rs->GetNextVariableName(&var_name_size, var_name_buf, &vendor_guid);
    while (status != EFI_NOT_FOUND) {   // End of list
        if (status == EFI_BUFFER_TOO_SMALL) {
            // Reallocate larger buffer for variable name
            CHAR16 *temp_buf = NULL;
            status = bs->AllocatePool(EfiLoaderData, var_name_size, (VOID **)&temp_buf);
            if (EFI_ERROR(status)) {
                error(status, u"Could not allocate %u bytes of memory for next variable name.\r\n",
                              var_name_size);
                return status;
            }
            
            strcpy_c16(temp_buf, var_name_buf);  // Copy old buffer to new buffer
            bs->FreePool(var_name_buf);          // Free old buffer
            var_name_buf = temp_buf;             // Set new buffer

            status = rs->GetNextVariableName(&var_name_size, var_name_buf, &vendor_guid);
            continue;
        }

        // Print variable name
        printf_c16(u"%.*s\r\n", var_name_size, var_name_buf);

        // Pause at bottom of screen
        if (cout->Mode->CursorRow >= text_rows-2) {
            printf_c16(u"Press any key to continue...\r\n");
            get_key();
            cout->ClearScreen(cout);
        }
        status = rs->GetNextVariableName(&var_name_size, var_name_buf, &vendor_guid);
    }

    // Free buffer when done
    bs->FreePool(var_name_buf);

    printf_c16(u"\r\nPress any key to go back...\r\n");
    get_key();
    return EFI_SUCCESS;
}

// ==========================================================
// Print Boot variable values and allow user to change them
// ==========================================================
EFI_STATUS change_boot_variables(void) { 
    // Get Device Path to Text protocol to print Load Option device/file paths
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID dpttp_guid = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *dpttp;
    status = bs->LocateProtocol(&dpttp_guid, NULL, (VOID **)&dpttp);
    if (EFI_ERROR(status)) {
        error(status, u"Could not locate Device Path To Text Protocol.\r\n");
        return status;
    }

    // Overall screen loop
    UINT32 boot_order_attributes = 0;
    while (true) {
        cout->ClearScreen(cout);

        UINTN var_name_size = 0;
        CHAR16 *var_name_buf = 0;
        EFI_GUID vendor_guid = {0};

        var_name_size = 2;
        status = bs->AllocatePool(EfiLoaderData, var_name_size, (VOID **)&var_name_buf);
        if (EFI_ERROR(status)) {
            error(status, u"Could not allocate 2 bytes...\r\n");
            return status;
        }

        // Set variable name to point to initial single null byte, to start off call to get list of
        //   variable names
        *var_name_buf = u'\0';

        status = rs->GetNextVariableName(&var_name_size, var_name_buf, &vendor_guid);
        while (status != EFI_NOT_FOUND) {   // End of list
            if (status == EFI_BUFFER_TOO_SMALL) {
                // Reallocate larger buffer for variable name
                CHAR16 *temp_buf = NULL;
                status = bs->AllocatePool(EfiLoaderData, var_name_size, (VOID **)&temp_buf);
                if (EFI_ERROR(status)) {
                    error(status, u"Could not allocate %u bytes of memory for next variable name.\r\n",
                                  var_name_size);
                    return status;
                }
                
                strcpy_c16(temp_buf, var_name_buf);  // Copy old buffer to new buffer
                bs->FreePool(var_name_buf);          // Free old buffer
                var_name_buf = temp_buf;             // Set new buffer

                status = rs->GetNextVariableName(&var_name_size, var_name_buf, &vendor_guid);
                continue;
            }

            // Print variable name and their value(s)
            if (!memcmp(var_name_buf, u"Boot", 8)) {
                printf_c16(u"\r\n%.*s: ", var_name_size, var_name_buf);

                // Get variable value
                UINT32 attributes = 0;
                UINTN data_size = 0;
                VOID *data = NULL;

                // Call first with 0 data size to get actual size needed
                rs->GetVariable(var_name_buf, &vendor_guid, &attributes, &data_size, NULL);

                status = bs->AllocatePool(EfiLoaderData, data_size, (VOID **)&data);
                if (EFI_ERROR(status)) {
                    error(status, u"Could not allocate %u bytes of memory for GetVariable().\r\n",
                                  data_size);
                    goto cleanup;
                }

                // Get actual data now with correct size
                rs->GetVariable(var_name_buf, &vendor_guid, &attributes, &data_size, data);
                if (data_size == 0) goto next;  // Skip this one if no data

                if (!memcmp(var_name_buf, u"BootOrder", 18)) {
                    boot_order_attributes = attributes; // Use if user sets new BootOrder value

                    // Print array of UINT16 values
                    UINT16 *p = data;

                    for (UINTN i = 0; i < data_size / 2; i++)
                        printf_c16(u"%#.4x,", *p++);   

                    printf_c16(u"\r\n");
                    goto next;
                }

                if (!memcmp(var_name_buf, u"BootOptionSupport", 34)) {
                    // Single UINT32 value
                    UINT32 *p = data;
                    printf_c16(u"%#.8x\r\n", *p);  
                    goto next;
                }

                if (!memcmp(var_name_buf, u"BootNext",    18) || 
                    !memcmp(var_name_buf, u"BootCurrent", 22)) {

                    // Single UINT16 value
                    UINT16 *p = data;
                    printf_c16(u"%#.4hx\r\n", *p); 
                    goto next;
                }

                if (isxdigit_c16(var_name_buf[4]) && var_name_size == 18) {  
                    // Boot#### load option: Name size = 8 CHAR16 chars * 2 bytes + CHAR16 null bytes
                    EFI_LOAD_OPTION *load_option = (EFI_LOAD_OPTION *)data;
                    CHAR16 *description = (CHAR16 *)((UINT8 *)data + sizeof(UINT32) + sizeof(UINT16));
                    printf_c16(u"%s\r\n", description);    

                    CHAR16 *p = description;
                    UINTN strlen =  0;
                    while (p[strlen]) strlen++;  
                    strlen++;                    // Skip null byte

                    EFI_DEVICE_PATH_PROTOCOL *file_path_list = 
                        (EFI_DEVICE_PATH_PROTOCOL *)(description + strlen); 

                    CHAR16 *device_path_text = 
                        dpttp->ConvertDevicePathToText(file_path_list, FALSE, FALSE);

                    printf_c16(u"Device Path: %s\r\n", device_path_text ? device_path_text : u"(null)");

                    UINT8 *optional_data = (UINT8 *)file_path_list + load_option->FilePathListLength;
                    UINTN optional_data_size = data_size - (optional_data - (UINT8 *)data);
                    if (optional_data_size > 0) {
                        printf_c16(u"Optional Data: 0x");
                        for (UINTN i = 0; i < optional_data_size; i++)
                            printf_c16(u"%.2hhx", optional_data[i]);

                        printf_c16(u"\r\n"); 
                    }
                    
                    goto next; 
                }

                printf_c16(u"\r\n");  // Unhandled Boot* variable, go on with space before next one

                next:
                bs->FreePool(data);
            }

            // Pause at bottom of screen
            if (cout->Mode->CursorRow >= text_rows-2) {
                printf_c16(u"Press any key to continue...\r\n");
                get_key();
                cout->ClearScreen(cout);
            }
            status = rs->GetNextVariableName(&var_name_size, var_name_buf, &vendor_guid);
        }

        // Allow user to change values
        printf_c16(u"Press '1' to change BootOrder, '2' to change BootNext, or other to go back...");
        EFI_INPUT_KEY key = get_key();
        if (key.UnicodeChar == u'1') {
            // Change BootOrder - set new array of UINT16 values
            #define MAX_BOOT_OPTIONS 10
            UINT16 option_array[MAX_BOOT_OPTIONS] = {0};
            UINTN new_option = 0;
            UINT16 num_options = 0;
            for (UINTN i = 0; i < MAX_BOOT_OPTIONS; i++) {
                printf_c16(u"\r\nBoot Option %u (0000-FFFF): ", i+1);
                if (!get_num(&new_option, 16)) break;    // Stop processing
                option_array[num_options++] = new_option; 
            }

            EFI_GUID guid = EFI_GLOBAL_VARIABLE_GUID;
            status = rs->SetVariable(u"BootOrder", 
                                     &guid,
                                     boot_order_attributes, 
                                     num_options*2, 
                                     option_array);
            if (EFI_ERROR(status)) 
                error(status, u"Could not Set new value for BootOrder.\r\n");

        } else if (key.UnicodeChar == u'2') {
            // Change BootNext value - set new UINT16
            printf_c16(u"\r\nBootNext value (0000-FFFF): ");
            UINTN value = 0;
            if (get_num(&value, 16)) {
                EFI_GUID guid = EFI_GLOBAL_VARIABLE_GUID;
                UINT32 attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS |
                              EFI_VARIABLE_RUNTIME_ACCESS;

                status = rs->SetVariable(u"BootNext", &guid, attr, 2, &value);
                if (EFI_ERROR(status)) 
                    error(status, u"Could not Set new value for BootNext.\r\n");
            }

        } else {
            bs->FreePool(var_name_buf);
            break;
        }

        cleanup:
        // Free buffers when done
        bs->FreePool(var_name_buf);
    }

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI ExitApp(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
    // Perform any final operations before exit, like cleanup or logging
    systemTable->ConOut->OutputString(gST->ConOut, L"Exiting application...\r\n");

    // Exit the application gracefully
    gBS->Exit(imageHandle, EFI_SUCCESS, 0, NULL);

    // Normally, we wouldn't reach here, but if you want to return an EFI_STATUS
    return EFI_SUCCESS;
}
