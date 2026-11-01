# Wanderer 0.9 hardware testing

## Author’s report

I have physically tested Wanderer 0.9 on my own M-VAVE FM-1. Installation from official V15 and from SLOOP both succeeded. I also successfully switched from Wanderer back to V15 and from Wanderer to SLOOP. Wanderer remains a work in progress.

## Confirmed firmware transitions

| Starting firmware | Destination | Result |
|---|---|---|
| Official V15 | Wanderer 0.9 | Successful on my FM-1 |
| SLOOP | Wanderer 0.9 | Successful on my FM-1 |
| Wanderer 0.9 | Official V15 | Successful rollback on my FM-1 |
| Wanderer 0.9 | SLOOP | Successful on my FM-1 |

All four routes above are confirmed. This report supersedes earlier hardware-untested wording and the intermediate statement that installation from another custom firmware into Wanderer was unconfirmed.

These results cover the Firmware Hub’s requested installation routes from stock V15 and another custom firmware, and leaving Wanderer again. They are an author report from one FM-1, not Hub approval. The exact SLOOP version and installation tool have not been recorded. Detailed per-feature results, measured worst-case audio timing, every flash operation, other device revisions and interrupted-update recovery are outside the recorded scope.

The compiled release remains unchanged: `LVM-Wanderer-0.9-FM1.fwsc`, 612,196 bytes, `FM-1_929`, SHA-256 `c6c0a2829a5c1fe5959cb0d302c4de152b97fe3f29e469c400d1fbaefa236dc9`.
