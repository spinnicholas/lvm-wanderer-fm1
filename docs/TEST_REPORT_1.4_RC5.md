# Wanderer 1.4 RC5 shortcut hint visibility

The HOME + EDIT shortcut reminder now appears only when ZOOM is selected on the Accessibility page, in both normal and Zoom views. Two drawing conditions change from RC4. All positions, font sizes, explanatory labels and shortcut behavior stay unchanged.

Candidate SHA-256: `25fd013fd515e9159dd407eedae0a4d197e66d13c82d47ae19fe343d72a95ec7`. Package: 612,196 bytes; identity FM-1_929.

Fresh verification: sanitizer-backed production capture generates all 296 views. Pixel comparison against RC4 confirms exactly eight changed captures: Theme and Motion in both themes and both sizes, with changes confined to removal of the shortcut footer. The other 288 captures, including Zoom-option views, are pixel-identical. The six representative Accessibility views were visually inspected. Target compilation, RAM/pool/register checks, protected loader/source/package-head checks and boot-stack audit pass. Image: 454,256 / 581,564 bytes; ordinary RAM: 63,856 / 98,304 bytes; pool: 315,392 / 344,064 bytes.

RC4's 53-check regression and compiled shortcut interaction test remain historical evidence; they were not repeated for these two drawing conditions. Physical FM-1 acceptance remains pending.
