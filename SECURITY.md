# Security Policy for CoreSys

## Supported Versions

CoreSys is an experimental low-level operating system. Security fixes are applied only to the most recent development branch unless otherwise stated.

| Version | Supported |
|---------|-----------|
| Latest (main) | Yes |
| Older commits | No |

Users are strongly encouraged to run the latest version when testing or developing.

---

## Reporting a Vulnerability

Because CoreSys operates at firmware and kernel level, security issues can have serious consequences. Vulnerabilities must be reported **privately** to FÈUE main email (feue.com1@outlook.com) and must not be disclosed in public issue trackers, discussions, or pull requests.

When reporting a security issue, include:

- A clear description of the vulnerability  
- Steps required to reproduce the issue  
- Potential impact (e.g., memory corruption, privilege escalation, data leak)  
- Logs, crash dumps, or screenshots if available  
- Your environment (compiler, firmware, emulator or hardware)

Reports should be sent directly to the project maintainers through a private communication channel designated by the repository owner.

---

## What to Expect

After submitting a report:

1. Maintainers will acknowledge receipt of the report.
2. The issue will be investigated and validated.
3. A fix will be developed and tested.
4. A security update may be released.
5. Public disclosure will occur only after a fix is available, when possible.

Response times may vary depending on complexity and maintainer availability.

---

## Scope

This policy applies to:

- The CoreSys bootloader
- Kernel components
- Memory management
- UEFI interaction code
- Build and deployment scripts

Third-party tools (such as QEMU or OVMF) must be reported to their respective projects.

---

## Security Best Practices for Contributors

When contributing to CoreSys:

- Avoid unsafe memory operations unless absolutely necessary  
- Validate all firmware-provided pointers and structures  
- Do not assume hardware or firmware behavior is correct  
- Prefer fail-safe behavior over undefined behavior  
- Clearly document any code that touches hardware or firmware interfaces  

Low-level bugs can become security vulnerabilities. Defensive programming is required.

---

## Disclosure Policy

Security vulnerabilities will be disclosed responsibly. The project will aim to:

- Fix the issue before public disclosure  
- Credit the reporter when appropriate  
- Document the vulnerability and mitigation steps  

---

CoreSys security depends on responsible research and coordinated disclosure. We appreciate contributions that improve the safety and reliability of the system.
