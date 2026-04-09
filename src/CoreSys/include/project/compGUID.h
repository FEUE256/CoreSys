#pragma once

int CompareGuid(const EFI_GUID* a, const EFI_GUID* b) {
    const unsigned char* p1 = (const unsigned char*)a;
    const unsigned char* p2 = (const unsigned char*)b;
    for (size_t i = 0; i < sizeof(EFI_GUID); i++) { // <- size_t
        if (p1[i] != p2[i]) return 0;
    }
    return 1;
}
