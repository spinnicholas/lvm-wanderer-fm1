# Wanderer 0.9 hardware testing

## Author’s report

I have physically tested Wanderer 0.9 on my own M-VAVE FM-1. I installed it from the official V15 firmware, successfully rolled back from Wanderer to V15, and also successfully switched from Wanderer to SLOOP. Wanderer remains a work in progress.

## Confirmed firmware transitions

| Starting firmware | Destination | Result |
|---|---|---|
| Official V15 | Wanderer 0.9 | Successful on my FM-1 |
| Wanderer 0.9 | Official V15 | Successful rollback on my FM-1 |
| Wanderer 0.9 | SLOOP | Successful on my FM-1 |

This report supersedes the earlier generic hardware-test note and hardware-untested wording in the original bundle. It does not establish the reverse path from SLOOP (or another custom firmware) into Wanderer. The exact SLOOP version and installation tool have not been recorded. Detailed per-feature results, measured worst-case audio timing, all flash operations, other device revisions and interrupted-update recovery are outside the recorded scope.

The Firmware Hub asks for installation from official V15 and from another custom firmware, as well as leaving the firmware again. Installation from V15 and leaving Wanderer for both V15 and SLOOP are confirmed. The other-custom-firmware-to-Wanderer route remains unconfirmed; the complete Hub checklist is not claimed.

The compiled release remains unchanged: `LVM-Wanderer-0.9-FM1.fwsc`, 612,196 bytes, `FM-1_929`, SHA-256 `c6c0a2829a5c1fe5959cb0d302c4de152b97fe3f29e469c400d1fbaefa236dc9`.
