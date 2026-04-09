# Furture Planing

This doc come not be 100 % real thing that will come to CoreSys! This doc will also not be good formated!

Rem:
in efi_lib.h when sf() is done uncomment // use sf() here and // sf(); lines

## error codes

64 bit

base:

\[Negative; 1 bit = 1 negative, 0 positive]
\[DefaultHex; 1 bit = x hex / b binary / t base ten]
\[Severity; 2 bits]
\[SubseverityInfo; 2 bits]
\[Subsystem; 10 bits]
\[Subsubsystem; 6 bits]
\[Mode; 4 bits]
\[Args; 8 bits]
\[ErrorCode; 28 bits]
\[OS\_Other; 2 bits]

in like maybe hmmm docs/error/README.md and feue256.github.io/CoreSys/EC.html (EC Error Codes)

## Just to rem

build -p ShellPkg/ShellPkg.dsc -a X64 -t GCC5

## Old CB

# 3. CBC (Cyber::Boot CMD)
- [ ] Implementera CBC som ett fullständigt textbaserat kommandoskal i KM.
- [ ] Lägg till filsystemsstöd (läs/skriv) för User Data Partition (UDP) i CBC.
- [ ] Implementera grundläggande kommandon för filhantering och systemkommandon.
- [ ] Implementera update/upgrade-kommando:
    - Hämtar senaste version och filer från URL som https://feue256.github.io/cb/v[version]/[filnamn]
    - Verifierar nedladdade filer (checksum eller signatur).
    - Uppdaterar systemet och informerar användaren.
- [ ] Implementera nätverksstöd och minimalistisk HTTP-klient för nedladdning i CBC.
- [ ] Säkerställ rollback/fallback vid misslyckad uppdatering.

# 4. Säkerhets- och privilegiesystem
- [ ] Implementera autentisering i CBC för admin-rättigheter.
- [ ] Skapa en separat *Admin-submeny* i KM som kräver lösenord eller doas-godkännande.
- [ ] När man är inne i admin-submenyn:
    - Alla kommandon körs med admin-rättigheter.
- [ ] Implementera doas <kommando> i vanliga CBC för engångs admin-körningar:
    - Kräver bekräftelse (yes/no).
    - Kräver autentisering (lösenord).
- [ ] Implementera timeout eller automatisk utloggning från admin-submenyn efter viss tid.
- [ ] Skydda känsliga funktioner (t.ex. "Read ESP Files", skriva till ESP) med admin-behörighet.

# 5. CBC-kommandon och funktioner
- [ ] Grundläggande kommandon för:
    - Filhantering på UDP
    - Systeminformation
    - Uppdatering (update/upgrade)
    - Processhantering (om tillämpligt)
    - Körning av GUI-program från submenu Programs
- [ ] Hantera fel och ge tydliga felmeddelanden på engelska.

# 6. UI/UX och flöde
- [ ] BM visar minimal meny, laddar kernel vid val.
- [ ] KM visar full meny inklusive CBC, Programs och Admin-submeny.
- [ ] CBC hanterar användarinmatning, kommandon och autentisering smidigt.
- [ ] Bekräftelsefrågor för kritiska operationer (yes/no).
- [ ] Feedback till användare vid lyckade och misslyckade operationer.

# 7. Övrigt
- [ ] Dokumentera alla menyer och kommandon tydligt.
- [ ] Planera och implementera backup- och återställningsrutiner för uppdateringar.
- [ ] Säkerställ att CB och CBC körs stabilt med rättighetsmodellen.
- [ ] Testa uppdateringsmekanismen noggrant i olika scenarier.
System Power

Shutdown

Reboot

Recovery Mode

1. Display / Graphics

Change Text Mode

Set Graphics Mode

Mouse Test

3. Storage / File Operations

Read ESP Files

Read UDP Files

Print Block IO Partition's Info

4. Boot / OS Management

Load Kernel

UEFI Boot Order

5. System Info

Print OS Info

Print Memory Map

Print Config Tables

Print Global EFI Variables

Print Current Time and Date

## new /src/public

new website over halt from old CB (CyberBoot)

/docs/public for Website help

## OS API

OS API for TC and c etc in CS (Technical C, CoreSys)

The OS API shell be wrieen in TC (Technical C)

/docs/api/ for docs

## C2 (CoreSys Complex Input Output System)

a OVMF fork in TC (Technical C)

ONLY VM support (because: I have a life (lol) and I dont have dual UEFI chip on my motherboard) (OK If i had daul chip I woald love to make hardware support but now it will not happen entill i uppgrade to daul UEFI chip on the motherbord)

Docs in /docs/c2

You can also call it CIOS also like you dont need to call it C2

like: 

C2 (CoreSys Complex Input Output System)

Map:
Boot button pressed ⇒ C2 ⇒ C2 Menu (Like the UEFI settings) ⇒ Reads the .os file ⇒ Bootloader ⇒ Pre-Kernel Menu ⇒ ExitBootSerives ⇒ ⇒ Reads .kernel file ⇒ Kernel/OS

File Types:
.os
.run
.tc
.kernal

C2 is written in asm, c and mostly 99 % TC.

Make a version of OVMF in TC. TC is a custom programming language

CoreSys will be default OS now todays its a .img file we will in furte realse .img and .os for c2 to boot 

With c2 Coresys will also be holly rewritten in tc and c2 will have preaks that UEFI dont

AT ANY time ctrl c is pressed it will shutdown if it downing something critual like idk something it will say DOING CRITICAL PROCESS EXITING CAN BRICK your computer press alt ctrl shift del fn c to shutdown

IDK just planning

\# PLAN 

* API
* Drivers (and UDAM (Unsigned Driver Allowed Mode) in bootloader)
* Pin/Password
* Std Drivers hardcoded signed public key / private
* TPM
* Finding CoreSys computer recurerments
