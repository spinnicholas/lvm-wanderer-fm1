"""Compare native production RGB565 splash frames with the supplied-art reference."""
from pathlib import Path
from PIL import Image
R=Path(__file__).resolve().parents[1]
reference=Image.open(R/'build/splash-art/Wanderer-New-Splash.png').convert('RGB').tobytes()
for theme in ['44-splash-dark','45-splash-light']:
 image=Image.open(R/'build/screens'/f'{theme}.ppm').convert('RGB');assert image.size==(240,240);assert image.tobytes()==reference
print('Both native splash themes match the full supplied artwork reference pixel for pixel')
